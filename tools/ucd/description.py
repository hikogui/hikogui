
import sys

class description (object):
    def __init__(self):
        # UnicodeData.txt
        self.bidi_class = "ON"
        self.canonical_combining_class = 0
        self.decomposition_type = None
        self.decomposition_mapping = []
        self.general_category = "Cn"
        self.line_break = "XX"
        self.bidi_mirrored = False
        self.upper_cast_mapping = []
        self.lower_cast_mapping = []
        self.title_cast_mapping = []

        # BidiBrackets.txt
        self.bidi_paired_bracket_type = "n"
        self.bidi_paired_bracket = None

        # BidiMirring.txt
        self.bidi_mirroring_glyph = None

        # CompositionExclusions.txt
        self.composition_exclusion = False

        # EastAsianWidth.txt
        self.east_asian_width = "N"

        # GraphemeBreakProperty.txt
        self.grapheme_cluster_break = "Other"

        # SentenceBreakProperty.txt
        self.sentence_break = "Other"

        # WordBreakProperty.txt
        self.word_break = "Other"

        # LineBreak.txt
        self.line_break = "XX"

        # Scripts.txt
        self.script = "Zzzz"

        # emoji-data.txt
        self.emoji = False
        self.emoji_presentation = False
        self.emoji_modifier = False
        self.emoji_modifier_base = False
        self.emoji_component = False
        self.extended_pictographic = False

    def __eq__(self, other):
        return (
            self.bidi_class == other.bidi_class and
            self.canonical_combining_class == other.canonical_combining_class and
            self.decomposition_type == other.decomposition_type and
            self.decomposition_mapping == other.decomposition_mapping and
            self.general_category == other.general_category and
            self.line_break == other.line_break and
            self.bidi_mirrored == other.bidi_mirrored and
            self.upper_cast_mapping == other.upper_cast_mapping and
            self.lower_cast_mapping == other.lower_cast_mapping and
            self.title_cast_mapping == other.title_cast_mapping and
            self.bidi_paired_bracket_type == other.bidi_paired_bracket_type and
            self.bidi_paired_bracket == other.bidi_paired_bracket and
            self.bidi_mirroring_glyph == other.bidi_mirroring_glyph and
            self.composition_exclusion == other.composition_exclusion and
            self.east_asian_width == other.east_asian_width and
            self.grapheme_cluster_break == other.grapheme_cluster_break and
            self.sentence_break == other.sentence_break and
            self.word_break == other.word_break and
            self.line_break == other.line_break and
            self.script == other.script and
            self.emoji == other.emoji and
            self.emoji_presentation == other.emoji_presentation and
            self.emoji_modifier == other.emoji_modifier and
            self.emoji_modifier_base == other.emoji_modifier_base and
            self.emoji_component == other.emoji_component and
            self.extended_pictographic == other.extended_pictographic
        )

    def general_category_as_integer(self):
        table = {
            "Lu": 0,
            "Ll": 1,
            "Lt": 2,
            "Lm": 3,
            "Lo": 4,
            "Mn": 5,
            "Mc": 6,
            "Me": 7,
            "Nd": 8,
            "Nl": 9,
            "No": 10,
            "Pc": 11,
            "Pd": 12,
            "Ps": 13,
            "Pe": 14,
            "Pi": 15,
            "Pf": 16,
            "Po": 17,
            "Sm": 18,
            "Sc": 19,
            "Sk": 20,
            "So": 21,
            "Zs": 22,
            "Zl": 23,
            "Zp": 24,
            "Cc": 25,
            "Cf": 26,
            "Cs": 27,
            "Co": 28,
            "Cn": 29
        }
        return table[self.general_category]

    def east_asian_width_as_integer(self):
        table = {
            "A": 0,
            "F": 1,
            "H": 2,
            "N": 3,
            "Na": 4,
            "W": 5
        }
        return table[self.east_asian_width]

    def bidi_paired_bracket_type_as_integer(self):
        table = {
            "n": 0,
            "o": 1,
            "c": 2,
            "m": 3
        }
        return table[self.bidi_paired_bracket_type]

    def bidi_class_as_integer(self):
        table = {
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
            "LRE": 15,
            "LRO": 16,
            "RLE": 17,
            "RLO": 18,
            "PDF": 19,
            "LRI": 20,
            "RLI": 21,
            "FSI": 22,
            "PDI": 23
        }
        return table[self.bidi_class]

    def grapheme_cluster_break_as_integer(self):
        table = {
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
        return table["Extended_Pictographic" if self.extended_pictographic else self.grapheme_cluster_break]

    def line_break_class_as_integer(self):
        table = {
            "BK": 0,
            "CR": 1,
            "LF": 2,
            "CM": 3,
            "NL": 4,
            "SG": 5,
            "WJ": 6,
            "ZW": 7,
            "GL": 8,
            "SP": 9,
            "ZWJ" : 10,
            "B2": 11,
            "BA": 12,
            "BB": 13,
            "HY": 14,
            "CB": 15,
            "CL": 16,
            "CP": 17,
            "EX": 18,
            "IN": 19,
            "NS": 20,
            "OP": 21,
            "QU": 22,
            "IS": 23,
            "NU": 24,
            "PO": 25,
            "PR": 26,
            "SY": 27,
            "AI": 28,
            "AL": 29,
            "CJ": 30,
            "EB": 31,
            "EM": 32,
            "H2": 33,
            "H3": 34,
            "HL": 35,
            "ID": 36,
            "JL": 37,
            "L": 38,
            "JV": 39,
            "V": 40,
            "JT": 41,
            "T": 42,
            "RI": 43,
            "SA": 44,
            "XX": 45
        }
        return table[self.line_break]

    def word_break_property_as_integer(self):
        table = {
            "Other": 0,
            "CR": 1,
            "LF": 2,
            "Newline": 3,
            "Extend": 4,
            "ZWJ": 5,
            "Regional_Indicator": 6,
            "Format": 7,
            "Katakana": 8,
            "Hebrew_Letter": 9,
            "ALetter": 10,
            "Single_Quote": 11,
            "Double_Quote": 12,
            "MidNumLet": 13,
            "MidLetter": 14,
            "MidNum": 15,
            "Numeric": 16,
            "ExtendNumLet": 17,
            "WSegSpace": 18
        }
        return table[self.word_break]

    def sentence_break_property_as_integer(self):
        table = {
            "Other": 0,
            "CR": 1,
            "LF": 2,
            "Extend": 3,
            "Sep": 4,
            "Format": 5,
            "Sp": 6,
            "Lower": 7,
            "Upper": 8,
            "OLetter": 9,
            "Numeric": 10,
            "ATerm": 11,
            "SContinue": 12,
            "STerm": 13,
            "Close": 14
        }
        return table[self.sentence_break]

    def decomposition_type_as_integer(self):
        types = {
            None: 0,
            "font": 1,
            "noBreak": 2,
            "initial": 3,
            "medial": 4,
            "final": 5,
            "isolated": 6,
            "circle": 7,
            "super": 8,
            "sub": 9,
            "fraction": 10,
            "vertical": 11,
            "wide": 12,
            "narrow": 13,
            "small": 14,
            "square": 15,
            "compat": 16
        }
        return types[self.decomposition_type]

    def instantiation(self):
        s = "XD{"
        s += "XSC::{}, ".format(self.script)
        s += "0x{:x} ".format(self.bidi_mirroring_glyph if self.bidi_mirroring_glyph is not None else 0xffff)
        s += "}"
        return s

def initialize_descriptions():
    """Initialize a table with description for all 0x110000 code-points.
    """
    print("Initialize descriptions for 0x110000 code-points.", file=sys.stderr, flush=True)
    return [description() for i in range(0x110000)]

