
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

    def grapheme_cluster_break_as_integer(self):
        gcbs = {
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
        return gcbs[self.grapheme_cluster_break]

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
        s += "XGC::{}, ".format(self.general_category)
        s += "XLB::{}, ".format(self.line_break)
        s += "XWB::{}, ".format(self.word_break)
        s += "XSB::{}, ".format(self.sentence_break)
        s += "XEA::{}, ".format(self.east_asian_width)
        s += "XSC::{}, ".format(self.script)
        s += "XBC::{}, ".format(self.bidi_class)
        s += "XBB::{}, ".format(self.bidi_paired_bracket_type)
        s += "0x{:x} ".format(self.bidi_mirroring_glyph if self.bidi_mirroring_glyph is not None else 0xffff)
        s += "}"
        return s

def initialize_descriptions():
    """Initialize a table with description for all 0x110000 code-points.
    """
    print("Initialize descriptions for 0x110000 code-points.", file=sys.stderr, flush=True)
    return [description() for i in range(0x110000)]

