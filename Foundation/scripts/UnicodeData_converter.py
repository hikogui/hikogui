#!/usr/bin/env python3

import sys
import os.path
import argparse
import struct

parser = argparse.ArgumentParser(description='Build binary from Unicode ucd text files.')
parser.add_argument("--output", dest="output_path", action="store", required=True)
parser.add_argument("--unicode-data", dest="unicode_data_path", action="store", required=True)
parser.add_argument("--emoji-data", dest="emoji_data_path", action="store", required=True)
parser.add_argument("--composition-exclusions", dest="composition_exclusions_path", action="store", required=True)
parser.add_argument("--grapheme-break-property", dest="grapheme_break_property_path", action="store", required=True)
options = parser.parse_args()


graphemeUnitTypes = {
    "Other": 0,
    "CR": 1,
    "LF": 2,
    "Control": 3,
    "Extend": 4,
    "ZWJ": 5,
    "Regional_Indicator": 6,
    "Prepend": 7,
    "SpacingMark": 8,
    "L": 9,
    "V": 10,
    "T": 11,
    "LV": 12,
    "LVT": 13,
    "Extended_Pictographic": 14
}

bidiClassValues = {
    # Explicit
    "LRE": 0,
    "LRO": 0,
    "RLE": 0,
    "RLO": 0,
    "PDF" : 0,
    "LRI" : 0,
    "RLI": 0,
    "FSI": 0,
    "PDI": 0,

    # Implicit
    "L": 1,
    "R": 2,
    "AL": 3,
    "EN": 4,
    "ES": 5,
    "ET": 6,
    "AN": 7,
    "CS": 8,
    "NSM": 9,
    "BN": 10,
    "B": 11,
    "S": 12,
    "WS": 13,
    "ON": 14,
}

class Composition (object):
    def __init__(self, description, startCodePoint, secondCodePoint, combinedCodePoint):
        self.description = description
        self.startCodePoint = startCodePoint
        self.secondCodePoint = secondCodePoint
        self.combinedCodePoint = combinedCodePoint

    def serialize(self):
        qword = self.startCodePoint << 43
        qword |= self.secondCodePoint << 22
        qword |= self.combinedCodePoint
        return struct.pack("<Q", qword)

class Decomposition (object):
    def __init__(self, description, decomposition):
        self.description = description
        self.decomposition = decomposition

    def serialize(self):
        ret = b""
        for i in range(0, len(self.decomposition), 3):
            qword = self.decomposition[i] << 43
            if i + 1 < len(self.decomposition):
                qword |= self.decomposition[i+1] << 22
            if i + 2 < len(self.decomposition):
                qword |= self.decomposition[i+2]
            ret += struct.pack("<Q", qword)

        return ret

class UnicodeDescription (object):
    def __init__(self, codePoint, decomposition, decompositionIsCanonical, decompositionOrder, bidiClass):
        self.codePoint = codePoint;
        self.decomposition = decomposition
        self.decompositionIsCanonical = decompositionIsCanonical
        self.decompositionOrder = decompositionOrder
        self.decompositionOffset = None
        self.graphemeUnitType = graphemeUnitTypes["Other"]
        self.bidiClass = bidiClass

    def serialize(self):
        decompositionFlagsAndLength = (
            len(self.decomposition) |
            (0x20 if self.decompositionIsCanonical else 0)
        )

        qword = self.codePoint << 43
        qword |= self.bidiClass << 39
        qword |= self.graphemeUnitType << 35
        qword |= (1 << 34) if self.decompositionIsCanonical else 0
        qword |= self.decompositionOrder << 26
        qword |= len(self.decomposition) << 21

        if len(self.decomposition) == 0:
            pass
        elif len(self.decomposition) == 1:
            qword |= self.decomposition[0]
        else:
            if self.decompositionOffset % 8 != 0:
                raise RuntimeError("Except decomposition offset to be a multiple of 8")
            qword |= (self.decompositionOffset // 8)

        return struct.pack("<Q", qword)
            
    def __repr__(self):
        return "code={}, offset={}, decomposition={}, canonical={}, order={}, graphemeUnitType={}".format(
            self.codePoint,
            self.decompositionOffset,
            self.decomposition,
            self.decompositionIsCanonical,
            self.decompositionOrder,
            self.graphemeUnitType,
        )

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
                descriptions[codePoint].graphemeUnitType = graphemeUnitTypes[columns[1]]

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
                    descriptions[codePoint].graphemeUnitType = graphemeUnitTypes["Extended_Pictographic"]

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
            decomposition=decomposition,
            decompositionIsCanonical=(decomposition_type is None),
            decompositionOrder=int(columns[3]),
            bidiClass=bidiClassValues[columns[4]]
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

def setDecompositionOffsets(descriptions, compositions, decompositions):
    offset = 16 # header size
    offset += len(descriptions) * 8
    
    for composition in compositions:
        composition.description.decompositionOffset = offset
        offset += 8

    for decomposition in decompositions:
        decomposition.description.decompositionOffset = offset
        # 3 code-points compacted into 64 bits.
        offset += ((len(decomposition.decomposition) + 2) // 3) * 8

def writeUnicodeData(filename, descriptions, compositions, decompositions):
    fd = open(filename, "wb")
    fd.write(struct.pack("<LLLL",
        0x62756364, # magic
        0x1, # version
        len(descriptions),
        len(compositions)
    ))

    for description in descriptions:
        fd.write(description.serialize())

    for composition in compositions:
        fd.write(composition.serialize())

    for decomposition in decompositions:
        fd.write(decomposition.serialize())

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

    checkDecompositionsForStartWithStart(descriptions)
    descriptions = sorted(descriptions.values(), key=lambda x: x.codePoint)

    compositions = extractCompositions(descriptions, composition_exclusions)
    decompositions = extractOtherDecompositions(descriptions, composition_exclusions)

    setDecompositionOffsets(descriptions, compositions, decompositions)

    writeUnicodeData(options.output_path, descriptions, compositions, decompositions)

if __name__ == "__main__":
    main()
