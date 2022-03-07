#!/usr/bin/env python3

import sys
import os.path
import argparse
import struct

parser = argparse.ArgumentParser(description='Build c++ source files from Unicode ucd text files.')
parser.add_argument("--output", dest="output_path", action="store", required=True)
parser.add_argument("--output-non-starter", dest="output_non_starter_path", action="store", required=True)
parser.add_argument("--unicode-data", dest="unicode_data_path", action="store", required=True)
parser.add_argument("--emoji-data", dest="emoji_data_path", action="store", required=True)
parser.add_argument("--composition-exclusions", dest="composition_exclusions_path", action="store", required=True)
parser.add_argument("--grapheme-break-property", dest="grapheme_break_property_path", action="store", required=True)
parser.add_argument("--bidi-brackets", dest="bidi_brackets_path", action="store", required=True)
parser.add_argument("--bidi-mirroring", dest="bidi_mirroring_path", action="store", required=True)
parser.add_argument("--line-break", dest="line_break_path", action="store", required=True)
parser.add_argument("--word-break", dest="word_break_path", action="store", required=True)
parser.add_argument("--sentence-break", dest="sentence_break_path", action="store", required=True)
parser.add_argument("--east-asian-width", dest="east_asian_width_path", action="store", required=True)
parser.add_argument("--scripts", dest="scripts_path", action="store", required=True)
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
    def __init__(self, codePoint, generalCategory, decomposition, decomposition_type, canonical_combining_class, bidiClass, bidiMirrored):
        self.codePoint = codePoint;
        self.generalCategory = generalCategory
        self.graphemeClusterBreak = "Other"
        self.bidiClass = bidiClass
        self.bidiMirrored = bidiMirrored
        self.bidiBracketType = "n"
        self.bidiMirroredGlyph = 0xffff
        self.decomposition = decomposition
        self.decomposition_type = decomposition_type
        self.compositionIsCanonical = False
        self.canonical_combining_class = canonical_combining_class
        self.decompositionIndex = None
        self.non_starter_code = 0
        self.lineBreakClass = "XX"
        self.word_break_property = "Other"
        self.sentence_break_property = "Other"
        self.eastAsianWidth = "N"
        self.script = "Unknown"

    def serialize(self):
        s = "TTXD{"

        # Generic character information
        s += format_char32(self.codePoint)
        s += ", TTXGC::{}".format(self.generalCategory)
        s += ", TTXGU::{}".format(self.graphemeClusterBreak)
        s += ", TTXLB::{}".format(self.lineBreakClass)
        s += ", TTXWB::{}".format(self.word_break_property)
        s += ", TTXSB::{}".format(self.sentence_break_property)
        s += ", TTXEA::{}".format(self.eastAsianWidth)
        s += ", TTXSC::{}".format(self.script)

        # Bidirection algorithm
        s += ", TTXBC::{}".format(self.bidiClass)
        if self.bidiBracketType == "n":
            s += ", TTXBB::m" if self.bidiMirrored else ", TTXBB::n"
        else:
            s += ", TTXBB::{}".format(self.bidiBracketType)
        s += ", " + format_char32(self.bidiMirroredGlyph)

        # Composition / Decomposition
        if self.decomposition_type == "canonical":
            s += ", TTXDT::canonical"
        elif self.decomposition_type == "font":
            s += ", TTXDT::font"
        elif self.decomposition_type == "noBreak":
            s += ", TTXDT::no_break"
        elif self.decomposition_type in ("initial", "medial", "final", "isolated"):
            s += ", TTXDT::arabic"
        elif self.decomposition_type == "circle":
            s += ", TTXDT::circle"
        elif self.decomposition_type in ("super", "sub", "fraction"):
            s += ", TTXDT::math"
        elif self.decomposition_type in ("vertical", "wide", "narrow", "small", "square"):
            s += ", TTXDT::asian"
        elif self.decomposition_type == "compat":
            s += ", TTXDT::compat"
        else:
            raise RuntimeError("Unknown decomposition_type '{}'".format(self.decomposition_type))

        # compositionIsCanonical the decomposition is available in the composition table.
        s += ", true" if self.compositionIsCanonical else ", false"
        s += ", {}".format(self.canonical_combining_class)
        s += ", {}".format(len(self.decomposition))

        if len(self.decomposition) == 0:
            s += ", 0"
        elif len(self.decomposition) == 1:
            s += ", {:#x}".format(self.decomposition[0])
        else:
            s += ", {}".format(self.decompositionIndex)

        s += ", {}".format(self.non_starter_code)

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

def parseLineBreak(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue

        columns = [x.strip() for x in line.split(";")]
        
        codePointRange = [int(x, 16) for x in columns[0].split("..")]
        if len(codePointRange) == 1:
            codePointRange.append(codePointRange[0])

        lineBreakClass = columns[1]
        for codePoint in range(codePointRange[0], codePointRange[1] + 1):
            if codePoint in descriptions:
                descriptions[codePoint].lineBreakClass = lineBreakClass

def parse_word_break_property(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue

        columns = [x.strip() for x in line.split(";")]
        
        code_point_range = [int(x, 16) for x in columns[0].split("..")]
        if len(code_point_range) == 1:
            code_point_range.append(code_point_range[0])

        word_break_property = columns[1]
        for code_point in range(code_point_range[0], code_point_range[1] + 1):
            if code_point in descriptions:
                descriptions[code_point].word_break_property = word_break_property

def parse_sentence_break_property(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue

        columns = [x.strip() for x in line.split(";")]
        
        code_point_range = [int(x, 16) for x in columns[0].split("..")]
        if len(code_point_range) == 1:
            code_point_range.append(code_point_range[0])

        sentence_break_property = columns[1]
        for code_point in range(code_point_range[0], code_point_range[1] + 1):
            if code_point in descriptions:
                descriptions[code_point].sentence_break_property = sentence_break_property


def parseEastAsianWidth(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue

        columns = [x.strip() for x in line.split(";")]
        
        codePointRange = [int(x, 16) for x in columns[0].split("..")]
        if len(codePointRange) == 1:
            codePointRange.append(codePointRange[0])

        eastAsianWidth = columns[1]
        for codePoint in range(codePointRange[0], codePointRange[1] + 1):
            if codePoint in descriptions:
                descriptions[codePoint].eastAsianWidth = eastAsianWidth

def parseScripts(filename, descriptions):
    for line in open(filename, encoding="utf-8"):
        line = line.rstrip()
        line = line.split("#", 1)[0]
        if line == "":
            continue

        columns = [x.strip() for x in line.split(";")]
        
        codePointRange = [int(x, 16) for x in columns[0].split("..")]
        if len(codePointRange) == 1:
            codePointRange.append(codePointRange[0])

        script = columns[1]
        for codePoint in range(codePointRange[0], codePointRange[1] + 1):
            if codePoint in descriptions:
                descriptions[codePoint].script = script

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
            decomposition_type = "canonical"
        else:
            if decomposition[0] == "<":
                decomposition_type, decomposition = decomposition[1:].split("> ")
            else:
                decomposition_type = "canonical"

            decomposition = [int(x, 16) for x in decomposition.split(" ")]

        description = UnicodeDescription(
            codePoint=int(columns[0], 16),
            generalCategory=columns[2],
            decomposition=decomposition,
            decomposition_type=decomposition_type,
            canonical_combining_class=int(columns[3]),
            bidiClass=columns[4],
            bidiMirrored=columns[9] == "Y"
        )

        descriptions[description.codePoint] = description

    return descriptions

def isCanonicalComposition(description, composition_exclusions):
    return (
        len(description.decomposition) == 2 and
        description.decomposition_type == "canonical" and
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

def extractNonStarters(descriptions):
    non_starters = []
    for description in descriptions:
        if description.canonical_combining_class != 0:
            description.non_starter_code = len(non_starters)
            non_starters.append(description.codePoint)

    return non_starters

def setDecompositionIndices(descriptions, compositions, decompositions):
    for index, composition in enumerate(compositions):
        composition.description.decompositionIndex = index

    index = 0
    for decomposition in decompositions:
        decomposition.description.decompositionIndex = index
        index += len(decomposition)

def writeUnicodeNonStarterData(filename, non_starters):
    fd = open(filename, "w")
    fd.write('// This file is generated by unicode_data_generator.py\n\n')

    fd.write('#include <array>\n\n')

    fd.write('namespace tt::inline v1::detail {\n\n')

    fd.write('constexpr auto unicode_db_non_starter_table = std::array{\n')
    fd.write('#ifndef __INTELLISENSE__\n')

    for i, code_point in enumerate(non_starters):
        if i < len(non_starters) - 1:
            fd.write('    {},\n'.format(format_char32(code_point)))
        else:
            fd.write('#endif\n')
            fd.write('    {}\n'.format(format_char32(code_point)))

    fd.write('};\n\n')

    fd.write('}\n')
    fd.close()

def writeUnicodeData(filename, descriptions, compositions, decompositions):
    fd = open(filename, "w")
    fd.write('// This file is generated by unicode_data_generator.py\n\n')

    fd.write('#include "unicode_general_category.hpp"\n')
    fd.write('#include "unicode_bidi_bracket_type.hpp"\n')
    fd.write('#include "unicode_bidi_class.hpp"\n')
    fd.write('#include "unicode_grapheme_cluster_break.hpp"\n')
    fd.write('#include "unicode_line_break.hpp"\n')
    fd.write('#include "unicode_word_break.hpp"\n')
    fd.write('#include "unicode_sentence_break.hpp"\n')
    fd.write('#include "unicode_east_asian_width.hpp"\n')
    fd.write('#include "unicode_script.hpp"\n')
    fd.write('#include "unicode_composition.hpp"\n')
    fd.write('#include "unicode_decomposition_type.hpp"\n')
    fd.write('#include "unicode_description.hpp"\n')
    fd.write('#include <array>\n\n')

    fd.write('namespace tt::inline v1::detail {\n\n')

    fd.write('#define TTXD unicode_description\n')
    fd.write('#define TTXGC unicode_general_category\n')
    fd.write('#define TTXBC unicode_bidi_class\n')
    fd.write('#define TTXBB unicode_bidi_bracket_type\n')
    fd.write('#define TTXGU unicode_grapheme_cluster_break\n')
    fd.write('#define TTXLB unicode_line_break_class\n')
    fd.write('#define TTXWB unicode_word_break_property\n')
    fd.write('#define TTXSB unicode_sentence_break_property\n')
    fd.write('#define TTXEA unicode_east_asian_width\n')
    fd.write('#define TTXSC unicode_script\n')
    fd.write('#define TTXDT unicode_decomposition_type\n')
    fd.write('constexpr auto unicode_db_description_table = std::array{\n')
    fd.write('#ifndef __INTELLISENSE__\n')
    for i, description in enumerate(descriptions):
        if i < len(descriptions) - 1:
            fd.write('    {},\n'.format(description.serialize()))
        else:
            fd.write('#endif\n')
            fd.write('    {}\n'.format(description.serialize()))

    fd.write('};\n\n')
    fd.write('#undef TTXD\n')
    fd.write('#undef TTXDT\n')
    fd.write('#undef TTXGC\n')
    fd.write('#undef TTXBC\n')
    fd.write('#undef TTXBB\n')
    fd.write('#undef TTXGU\n')
    fd.write('#undef TTXLB\n')
    fd.write('#undef TTXWB\n')
    fd.write('#undef TTXSB\n')
    fd.write('#undef TTXEA\n')
    fd.write('#undef TTXSC\n')

    fd.write('#define TTXC unicode_composition\n')
    fd.write('constexpr auto unicode_db_composition_table = std::array{\n')
    fd.write('#ifndef __INTELLISENSE__\n')
    for i, composition in enumerate(compositions):
        if i < len(compositions) - 1:
            fd.write('    {},\n'.format(composition.serialize()))
        else:
            fd.write('#endif\n')
            fd.write('    {}\n'.format(composition.serialize()))

    fd.write('};\n\n')
    fd.write('#undef TTXC\n')

    decomposition_characters = []
    for decomposition in decompositions:
        for c in decomposition:
            decomposition_characters.append(c)

    fd.write('constexpr auto unicode_db_decomposition_table = std::array{\n')
    fd.write('#ifndef __INTELLISENSE__\n')
    for i, decomposition in enumerate(decompositions):
        if i < len(decompositions) - 1:
            fd.write('    {},\n'.format(decomposition.serialize()))
        else:
            fd.write('#endif\n')
            fd.write('    {}\n'.format(decomposition.serialize()))

    fd.write('};\n\n')

    fd.write('}\n')
    fd.close()

def checkDecompositionsForStartWithStart(descriptions):
    for description in descriptions.values():
        if len(description.decomposition) >= 1:
            firstDecompositionCodePoint = description.decomposition[0]
            if firstDecompositionCodePoint in descriptions:
                firstDecompositionCodePointDescription = descriptions[description.decomposition[0]]
                description.decompositionStartsWithStart = (firstDecompositionCodePointDescription.canonical_combining_class == 0)
            else:
                #raise RuntimeError("Missing %x" % (firstDecompositionCodePoint))
                description.decompositionStartsWithStart = True

def get_max_mirror_delta(descriptions):
    max_mirror_delta = 0
    for description in descriptions:
        if (description.bidiBracketType != 'n' or description.bidiMirrored) and description.bidiMirroredGlyph != 0xffff:
            delta = description.bidiMirroredGlyph - description.codePoint
            if max_mirror_delta < abs(delta):
                max_mirror_delta = abs(delta)

    return max_mirror_delta


def main():
    descriptions = parseUnicodeData(options.unicode_data_path)
    composition_exclusions = parseCompositionExclusions(options.composition_exclusions_path)
    parseGraphemeBreakProperty(options.grapheme_break_property_path, descriptions)
    parseLineBreak(options.line_break_path, descriptions)
    parse_word_break_property(options.word_break_path, descriptions)
    parse_sentence_break_property(options.sentence_break_path, descriptions)
    parseEastAsianWidth(options.east_asian_width_path, descriptions)
    parseScripts(options.scripts_path, descriptions)
    parseEmojiData(options.emoji_data_path, descriptions)
    parseBidiBrackets(options.bidi_brackets_path, descriptions)
    parseBidiMirroring(options.bidi_mirroring_path, descriptions)

    checkDecompositionsForStartWithStart(descriptions)
    descriptions = sorted(descriptions.values(), key=lambda x: x.codePoint)

    compositions = extractCompositions(descriptions, composition_exclusions)
    decompositions = extractOtherDecompositions(descriptions, composition_exclusions)
    non_starters = extractNonStarters(descriptions)

    setDecompositionIndices(descriptions, compositions, decompositions)

    writeUnicodeData(options.output_path, descriptions, compositions, decompositions)
    writeUnicodeNonStarterData(options.output_non_starter_path, non_starters)

    max_mirror_delta = get_max_mirror_delta(descriptions)
    print("Maximum delta between two mirroring code-points = {}".format(max_mirror_delta))

if __name__ == "__main__":
    main()
