#!/usr/bin/env python3

import sys
import os.path
import argparse
import struct

parser = argparse.ArgumentParser(description='Build c++ source files from Unicode ucd text files.')
parser.add_argument("--output", dest="output_path", action="store", required=True)
parser.add_argument("--unicode-data", dest="unicode_data_path", action="store", required=True)
parser.add_argument("--emoji-data", dest="emoji_data_path", action="store", required=True)
parser.add_argument("--composition-exclusions", dest="composition_exclusions_path", action="store", required=True)
parser.add_argument("--grapheme-break-property", dest="grapheme_break_property_path", action="store", required=True)
parser.add_argument("--bidi-brackets", dest="bidi_brackets_path", action="store", required=True)
parser.add_argument("--bidi-mirroring", dest="bidi_mirroring_path", action="store", required=True)
options = parser.parse_args()

def format_char32(x):
    if 0xd800 <= x <= 0xdfff:
        return "char32_t{{{:#x}}}".format(x)
    elif x <= 0xffff:
        return "U'\\u{:04x}'".format(x)
    else:
        return "U'\\U{:08x}'".format(x)

class Composition (object):
    def __init__(self, description, startCodePoint, secondCodePoint, combinedCodePoint):
        self.description = description
        self.startCodePoint = startCodePoint
        self.secondCodePoint = secondCodePoint
        self.combinedCodePoint = combinedCodePoint

    def serialize(self):
        s = "TTXC{"
        s += format_char32(self.startCodePoint)
        s += ","
        s += format_char32(self.secondCodePoint)
        s += ","
        s += format_char32(self.combinedCodePoint)
        s += "}"
        return s

class Decomposition (object):
    def __init__(self, description, decomposition):
        self.description = description
        self.decomposition = decomposition

    def __len__(self):
        return len(self.decomposition)

    def __getitem__(self, i):
        return self.decomposition[i]

    def serialize(self):
        return ",".join([format_char32(x) for x in self.decomposition])

class UnicodeDescription (object):
    def __init__(self, codePoint, generalCategory, decomposition, decompositionIsCanonical, decompositionOrder, bidiClass, bidiMirrored):
        self.codePoint = codePoint;
        self.generalCategory = generalCategory
        self.graphemeClusterBreak = "Other"
        self.bidiClass = bidiClass
        self.bidiMirrored = bidiMirrored
        self.bidiBracketType = "n"
        self.bidiMirroredGlyph = 0xffff
        self.decomposition = decomposition
        self.decompositionIsCanonical = decompositionIsCanonical
        self.compositionIsCanonical = False
        self.decompositionOrder = decompositionOrder
        self.decompositionIndex = None

    def serialize(self):
        s = "TTXD{"

        decompositionFlagsAndLength = (
            len(self.decomposition) |
            (0x20 if self.decompositionIsCanonical else 0)
        )

        # Generic character information
        s += format_char32(self.codePoint)
        s += ", TTXGC::{}".format(self.generalCategory)
        s += ", TTXGU::{}".format(self.graphemeClusterBreak)

        # Bidirection algorithm
        s += ", TTXBC::{}".format(self.bidiClass)
        if self.bidiBracketType == "n":
            s += ", TTXBB::m" if self.bidiMirrored else ", TTXBB::n"
        else:
            s += ", TTXBB::{}".format(self.bidiBracketType)
        s += ", " + format_char32(self.bidiMirroredGlyph)

        # Composition / Decomposition
        s += ", true" if self.decompositionIsCanonical else ", false"
        # compositionIsCanonical the decomposition is available in the composition table.
        s += ", true" if self.compositionIsCanonical else ", false"
        s += ", {}".format(self.decompositionOrder)
        s += ", {}".format(len(self.decomposition))

        if len(self.decomposition) == 0:
            s += ", 0"
        elif len(self.decomposition) == 1:
            s += ", {:#x}".format(self.decomposition[0])
        else:
            s += ", {}".format(self.decompositionIndex)

        s += "}"
        return s
            
def parseGraphemeBreakProperty(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue

        columns = [x.strip() for x in line.split(";")]

        codePoints = [int(x, 16) for x in columns[0].split("..")]
        if len(codePoints) == 1:
            codePoints.append(codePoints[0])

        for codePoint in range(codePoints[0], codePoints[1] + 1):
            if codePoint in descriptions:
                descriptions[codePoint].graphemeClusterBreak = columns[1]

def parseEmojiData(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue

        columns = [x.strip() for x in line.split(";")]
        
        codePointRange = [int(x, 16) for x in columns[0].split("..")]
        if len(codePointRange) == 1:
            codePointRange.append(codePointRange[0])

        emojiType = columns[1]
        if emojiType == "Extended_Pictographic":
            for codePoint in range(codePointRange[0], codePointRange[1] + 1):
                if codePoint in descriptions:
                    descriptions[codePoint].graphemeClusterBreak = "Extended_Pictographic"

def parseCompositionExclusions(filename):
    compositionExclusions = set()
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue
    
        codePoint = int(line.strip(), 16)
        compositionExclusions.add(codePoint)

    return compositionExclusions

def parseBidiBrackets(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue
    
        columns = [x.strip() for x in line.split(";")]
        codePoint = int(columns[0], 16)
        bidiBracketType = columns[2]
        descriptions[codePoint].bidiBracketType = bidiBracketType

def parseBidiMirroring(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue
    
        columns = [x.strip() for x in line.split(";")]
        codePoint = int(columns[0], 16)
        bidiMirrorredGlyph = int(columns[1], 16)
        descriptions[codePoint].bidiMirroredGlyph = bidiMirrorredGlyph

def parseUnicodeData(filename):
    descriptions = {}
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue

        columns = [x.strip() for x in line.split(";")]

        decomposition = columns[5]
        if not decomposition:
            decomposition = []
            decomposition_type = None
        else:
            if decomposition[0] == "<":
                decomposition_type, decomposition = decomposition[1:].split("> ")
            else:
                decomposition_type = None

            decomposition = [int(x, 16) for x in decomposition.split(" ")]

        description = UnicodeDescription(
            codePoint=int(columns[0], 16),
            generalCategory=columns[2],
            decomposition=decomposition,
            decompositionIsCanonical=(decomposition_type is None),
            decompositionOrder=int(columns[3]),
            bidiClass=columns[4],
            bidiMirrored=columns[9] == "Y"
        )

        descriptions[description.codePoint] = description

    return descriptions

def isCanonicalComposition(description, composition_exclusions):
    return (
        len(description.decomposition) == 2 and
        description.decompositionIsCanonical and
        description.codePoint not in composition_exclusions and
        description.decompositionStartsWithStart
    )

def extractCompositions(descriptions, composition_exclusions):
    compositions = []
    for description in descriptions:
        if isCanonicalComposition(description, composition_exclusions):
            description.compositionIsCanonical = True;
            composition = Composition(
                description=description,
                startCodePoint=description.decomposition[0],
                secondCodePoint=description.decomposition[1],
                combinedCodePoint=description.codePoint
            )
            compositions.append(composition)

    compositions.sort(key=lambda x: (x.startCodePoint, x.secondCodePoint))
    return compositions

def extractOtherDecompositions(descriptions, composition_exclusions):
    decompositions = []
    for description in descriptions:
        if len(description.decomposition) >= 2 and not isCanonicalComposition(description, composition_exclusions):
            decomposition = Decomposition(
                description=description,
                decomposition=description.decomposition
            )
            decompositions.append(decomposition)

    return decompositions

def setDecompositionIndices(descriptions, compositions, decompositions):
    for index, composition in enumerate(compositions):
        composition.description.decompositionIndex = index

    index = 0
    for decomposition in decompositions:
        decomposition.description.decompositionIndex = index
        index += len(decomposition)

def writeUnicodeData(filename, descriptions, compositions, decompositions):
    fd = open(filename, "w")
    fd.write('// This file is generated by unicode_data_generator.py\n\n')

    fd.write('#include "ttauri/text/unicode_general_category.hpp"\n')
    fd.write('#include "ttauri/text/unicode_bidi_bracket_type.hpp"\n')
    fd.write('#include "ttauri/text/unicode_bidi_class.hpp"\n')
    fd.write('#include "ttauri/text/unicode_grapheme_cluster_break.hpp"\n')
    fd.write('#include "ttauri/text/unicode_composition.hpp"\n')
    fd.write('#include "ttauri/text/unicode_description.hpp"\n')
    fd.write('#include <array>\n\n')

    fd.write('namespace tt::detail {\n\n')

    fd.write('#define TTXD unicode_description\n')
    fd.write('#define TTXGC unicode_general_category\n')
    fd.write('#define TTXBC unicode_bidi_class\n')
    fd.write('#define TTXBB unicode_bidi_bracket_type\n')
    fd.write('#define TTXGU unicode_grapheme_cluster_break\n')
    fd.write('constexpr auto unicode_db_description_table = std::array{')
    for i, description in enumerate(descriptions):
        if i != 0:
            fd.write(",")
        fd.write("\n    ")
        fd.write(description.serialize())
    fd.write('};\n\n')
    fd.write('#undef TTXD\n')
    fd.write('#undef TTXGC\n')
    fd.write('#undef TTXBC\n')
    fd.write('#undef TTXBB\n')
    fd.write('#undef TTXGU\n')

    fd.write('#define TTXC unicode_composition\n')
    fd.write('constexpr auto unicode_db_composition_table = std::array{')
    for i, composition in enumerate(compositions):
        if i != 0:
            fd.write(",")
        fd.write("\n    ")
        fd.write(composition.serialize())
    fd.write('};\n\n')
    fd.write('#undef TTXC\n')

    decomposition_characters = []
    for decomposition in decompositions:
        for c in decomposition:
            decomposition_characters.append(c)

    fd.write('constexpr auto unicode_db_decomposition_table = std::array{')
    for i, decomposition in enumerate(decompositions):
        if i != 0:
            fd.write(",")
        fd.write("\n    ")
        fd.write(decomposition.serialize())
    fd.write('};\n\n')

    fd.write('}\n')
    fd.close()

def checkDecompositionsForStartWithStart(descriptions):
    for description in descriptions.values():
        if len(description.decomposition) >= 1:
            firstDecompositionCodePoint = description.decomposition[0]
            if firstDecompositionCodePoint in descriptions:
                firstDecompositionCodePointDescription = descriptions[description.decomposition[0]]
                description.decompositionStartsWithStart = (firstDecompositionCodePointDescription.decompositionOrder == 0)
            else:
                #raise RuntimeError("Missing %x" % (firstDecompositionCodePoint))
                description.decompositionStartsWithStart = True

def main():
    descriptions = parseUnicodeData(options.unicode_data_path)
    composition_exclusions = parseCompositionExclusions(options.composition_exclusions_path)
    parseGraphemeBreakProperty(options.grapheme_break_property_path, descriptions)
    parseEmojiData(options.emoji_data_path, descriptions)
    parseBidiBrackets(options.bidi_brackets_path, descriptions)
    parseBidiMirroring(options.bidi_mirroring_path, descriptions)

    checkDecompositionsForStartWithStart(descriptions)
    descriptions = sorted(descriptions.values(), key=lambda x: x.codePoint)

    compositions = extractCompositions(descriptions, composition_exclusions)
    decompositions = extractOtherDecompositions(descriptions, composition_exclusions)

    setDecompositionIndices(descriptions, compositions, decompositions)

    writeUnicodeData(options.output_path, descriptions, compositions, decompositions)

if __name__ == "__main__":
    main()
