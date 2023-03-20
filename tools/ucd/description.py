
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

    def script_as_integer(self):
        table = {
            "Zzzz": 0,
            "Common": 1,
            "Latin": 2,
            "Greek": 3,
            "Cyrillic": 4,
            "Armenian": 5,
            "Hebrew": 6,
            "Arabic": 7,
            "Syriac": 8,
            "Thaana": 9,
            "Devanagari": 10,
            "Bengali": 11,
            "Gurmukhi": 12,
            "Gujarati": 13,
            "Oriya": 14,
            "Tamil": 15,
            "Telugu": 16,
            "Kannada": 17,
            "Malayalam": 18,
            "Sinhala": 19,
            "Thai": 20,
            "Lao": 21,
            "Tibetan": 22,
            "Myanmar": 23,
            "Georgian": 24,
            "Hangul": 25,
            "Ethiopic": 26,
            "Cherokee": 27,
            "Canadian_Aboriginal": 28,
            "Ogham": 29,
            "Runic": 30,
            "Khmer": 31,
            "Mongolian": 32,
            "Hiragana": 33,
            "Katakana": 34,
            "Bopomofo": 35,
            "Han": 36,
            "Yi": 37,
            "Old_Italic": 38,
            "Gothic": 39,
            "Deseret": 40,
            "Inherited": 41,
            "Tagalog": 42,
            "Hanunoo": 43,
            "Buhid": 44,
            "Tagbanwa": 45,
            "Limbu": 46,
            "Tai_Le": 47,
            "Linear_B": 48,
            "Ugaritic": 49,
            "Shavian": 50,
            "Osmanya": 51,
            "Cypriot": 52,
            "Braille": 53,
            "Buginese": 54,
            "Coptic": 55,
            "New_Tai_Lue": 56,
            "Glagolitic": 57,
            "Tifinagh": 58,
            "Syloti_Nagri": 59,
            "Old_Persian": 60,
            "Kharoshthi": 61,
            "Balinese": 62,
            "Cuneiform": 63,
            "Phoenician": 64,
            "Phags_Pa": 65,
            "Nko": 66,
            "Sundanese": 67,
            "Lepcha": 68,
            "Ol_Chiki": 69,
            "Vai": 70,
            "Saurashtra": 71,
            "Kayah_Li": 72,
            "Rejang": 73,
            "Lycian": 74,
            "Carian": 75,
            "Lydian": 76,
            "Cham": 77,
            "Tai_Tham": 78,
            "Tai_Viet": 79,
            "Avestan": 80,
            "Egyptian_Hieroglyphs": 81,
            "Samaritan": 82,
            "Lisu": 83,
            "Bamum": 84,
            "Javanese": 85,
            "Meetei_Mayek": 86,
            "Imperial_Aramaic": 87,
            "Old_South_Arabian": 88,
            "Inscriptional_Parthian": 89,
            "Inscriptional_Pahlavi": 90,
            "Old_Turkic": 91,
            "Kaithi": 92,
            "Batak": 93,
            "Brahmi": 94,
            "Mandaic": 95,
            "Chakma": 96,
            "Meroitic_Cursive": 97,
            "Meroitic_Hieroglyphs": 98,
            "Miao": 99,
            "Sharada": 100,
            "Sora_Sompeng": 101,
            "Takri": 102,
            "Caucasian_Albanian": 103,
            "Bassa_Vah": 104,
            "Duployan": 105,
            "Elbasan": 106,
            "Grantha": 107,
            "Pahawh_Hmong": 108,
            "Khojki": 109,
            "Linear_A": 110,
            "Mahajani": 111,
            "Manichaean": 112,
            "Mende_Kikakui": 113,
            "Modi": 114,
            "Mro": 115,
            "Old_North_Arabian": 116,
            "Nabataean": 117,
            "Palmyrene": 118,
            "Pau_Cin_Hau": 119,
            "Old_Permic": 120,
            "Psalter_Pahlavi": 121,
            "Siddham": 122,
            "Khudawadi": 123,
            "Tirhuta": 124,
            "Warang_Citi": 125,
            "Ahom": 126,
            "Anatolian_Hieroglyphs": 127,
            "Hatran": 128,
            "Multani": 129,
            "Old_Hungarian": 130,
            "SignWriting": 131,
            "Adlam": 132,
            "Bhaiksuki": 133,
            "Marchen": 134,
            "Newa": 135,
            "Osage": 136,
            "Tangut": 137,
            "Masaram_Gondi": 138,
            "Nushu": 139,
            "Soyombo": 140,
            "Zanabazar_Square": 141,
            "Dogra": 142,
            "Gunjala_Gondi": 143,
            "Makasar": 144,
            "Medefaidrin": 145,
            "Hanifi_Rohingya": 146,
            "Sogdian": 147,
            "Old_Sogdian": 148,
            "Elymaic": 149,
            "Nandinagari": 150,
            "Nyiakeng_Puachue_Hmong": 151,
            "Wancho": 152,
            "Chorasmian": 153,
            "Dives_Akuru": 154,
            "Khitan_Small_Script": 155,
            "Yezidi": 156,
            "Cypro_Minoan": 157,
            "Old_Uyghur": 158,
            "Tangsa": 159,
            "Toto": 160,
            "Vithkuqi": 161,
            "Afaka": 162,
            "Arabic_Nastaliq": 163,
            "Blissymbols": 164,
            "Cirth": 165,
            "Cyrillic_Old_Church_Slavonic": 166,
            "Egyptian_Demotic": 167,
            "Egyptian_Hieratic": 168,
            "Han_Bopomofo": 169,
            "Han_Simplified": 170,
            "Han_Traditional": 171,
            "Indus": 172,
            "Jamo": 173,
            "Japanese": 174,
            "Japanese_Syllabaries": 175,
            "Jurchen": 176,
            "Kawi": 177,
            "Khitan_Large_Script": 178,
            "Khutsuri": 179,
            "Korean": 180,
            "Kpelle": 181,
            "Latin_Fraktur": 182,
            "Latin_Gaelic": 183,
            "Leke": 184,
            "Loma": 185,
            "Mayan_Hieroglyphs": 186,
            "Moon": 187,
            "Nag_Mundari": 188,
            "Naxi_Dongba": 189,
            "Nakhi_Geba": 190,
            "Proto_Cuneiform": 191,
            "Proto_Elamite": 192,
            "Book_Pahlavi": 193,
            "Kligon": 194,
            "Proto_Sinaitic": 195,
            "Private_Use_aa": 196,
            "Private_Use_bx": 197,
            "Ranjana": 198,
            "Rongorongo": 199,
            "Sarati": 200,
            "Shuishu": 201,
            "Sunuwar": 202,
            "Symbols": 203,
            "Symbols_Emoji": 204,
            "Syriac_Estrangelo": 205,
            "Syriac_Western": 206,
            "Syriac_Eastern": 207,
            "Tengwar": 208,
            "Unwritten_Documents": 209,
            "Visible_Speech": 210,
            "Woleai": 211,
            "Mathematical_Notation": 212,
            "Byzantine_Music": 213,
            "Music": 214
        }
        return table[self.script]

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


def initialize_descriptions():
    """Initialize a table with description for all 0x110000 code-points.
    """
    print("Initialize descriptions for 0x110000 code-points.", file=sys.stderr, flush=True)
    return [description() for i in range(0x110000)]

