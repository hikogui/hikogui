#!/usr/bin/env python3

import sys
import os.path
import argparse
import struct

parser = argparse.ArgumentParser(description='Build binary from Unicode ucd text files.')
parser.add_argument("--output", dest="output_path", action="store", required=True)
parser.add_argument("--unicode-data", dest="unicode_data_path", action="store", required=True)
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
    "Extended Pictographic": 14
}

class Composition (object):
    def __init__(self, description, startCodePoint, secondCodePoint, combinedCodePoint):
        self.description = description
        self.startCodePoint = startCodePoint
        self.secondCodePoint = secondCodePoint
        self.combinedCodePoint = combinedCodePoint

    def serialize(self):
        return struct.pack("<LLL",
            self.startCodePoint,
            self.secondCodePoint,
            self.combinedCodePoint
        )

class Decomposition (object):
    def __init__(self, description, decomposition):
        self.description = description
        self.decomposition = decomposition

    def serialize(self):
        ret = b""
        for codePoint in decomposition:
            ret += struct.pack("<L", codePoint)
        return ret


class UnicodeDescription (object):
    def __init__(self, codePoint, decomposition, decompositionIsCanonical, decompositionOrder):
        self.codePoint = codePoint;
        self.decomposition = decomposition
        self.decompositionIsCanonical = decompositionIsCanonical
        self.decompositionOrder = decompositionOrder
        self.decompositionOffset = None
        self.graphemeUnitType = graphemeUnitTypes["Other"]

    def serialize(self):
        decompositionFlagsAndLength = (
            len(self.decomposition) |
            (0x20 if self.decompositionIsCanonical else 0)
        )

        return struct.pack("<LLBBBB",
            self.codePoint,
            self.decompositionOffset or 0,
            decompositionFlagsAndLength,
            self.decompositionOrder,
            self.graphemeUnitType,
            0
        )
            
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
            decompositionOrder=int(columns[3])
        )

        descriptions[description.codePoint] = description

    return descriptions

def extractCompositions(descriptions):
    compositions = []
    for description in descriptions:
        if description.decompositionIsCanonical and len(description.decomposition) == 2:
            composition = Composition(
                description=description,
                startCodePoint=description.decomposition[0],
                secondCodePoint=description.decomposition[1],
                combinedCodePoint=description.codePoint
            )
            compositions.append(composition)

    compositions.sort(key=lambda x: (x.startCodePoint, x.secondCodePoint))
    return compositions

def extractOtherDecompositions(descriptions):
    decompositions = []
    for description in descriptions:
        if (
            len(description.decomposition) > 0 and
            (not description.decompositionIsCanonical or len(description.decomposition) != 2)
        ):
            decomposition = Decomposition(
                description=description,
                decomposition=description.decomposition
            )
    return decompositions

def setDecompositionOffsets(descriptions, compositions, decompositions):
    offset = 16 # header size
    offset += len(descriptions) * 12
    
    for composition in compositions:
        composition.description.decompositionOffset = offset
        offset += 12

    for decomposition in decompositions:
        decomposition.description.decompositionOffset = offset
        offset += len(decomposition.decomposition) * 4

def writeBinaryUnicodeData(filename, descriptions, compositions, decompositions):
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
        fd.write(decompositions.serialize())

    fd.close()

def main():
    descriptions = parseUnicodeData(options.unicode_data_path)
    parseGraphemeBreakProperty(options.grapheme_break_property_path, descriptions)

    descriptions = sorted(descriptions.values(), key=lambda x: x.codePoint)

    compositions = extractCompositions(descriptions)
    decompositions = extractOtherDecompositions(descriptions)

    setDecompositionOffsets(descriptions, compositions, decompositions)

    writeBinaryUnicodeData(options.output_path, descriptions, compositions, decompositions)

if __name__ == "__main__":
    main()
    print("success?")
    sys.exit(2)
