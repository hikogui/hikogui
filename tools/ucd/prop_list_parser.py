
from .ucd_parser import parse_ucd

def parse_prop_list(filename, descriptions):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        t = columns[1]

        for code_point in code_points:
            d = descriptions[code_point]
            if t == "White_Space":
                d.white_space = True
            elif t == "Bidi_Control":
                d.bidi_control = True
            elif t == "Join_Control":
                d.join_control = True
            elif t == "Dash":
                d.dash = True
            elif t == "Hyphen":
                d.hyphen = True
            elif t == "Quotation_Mark":
                d.quotation_mark = True
            elif t == "Terminal_Punctuation":
                d.terminal_punctuation = True
            elif t == "Other_Math":
                d.other_math = True
            elif t == "Hex_Digit":
                d.hex_digit = True
            elif t == "ASCII_Hex_Digit":
                d.ascii_hex_digit = True
            elif t == "Other_Alphabetic":
                d.other_alphabetic = True
            elif t == "Ideographic":
                d.ideographic = True
            elif t == "Diacritic":
                d.diacritic = True
            elif t == "Extender":
                d.extender = True
            elif t == "Other_Lowercase":
                d.other_lowercase = True
            elif t == "Other_Uppercase":
                d.other_uppercase = True
            elif t == "Noncharacter_Code_Point":
                d.non_character = True
            elif t == "Other_Grapheme_Extend":
                d.other_grapheme_extend = True
            elif t == "IDS_Binary_Operator":
                d.ids_binary_operator = True
            elif t == "IDS_Trinary_Operator":
                d.ids_trinary_operator = True
            elif t == "Radical":
                d.radical = True
            elif t == "Unified_Ideograph":
                d.unified_ideograph = True
            elif t == "Other_Default_Ignorable_Code_Point":
                d.other_default_ignorable = True
            elif t == "Deprecated":
                d.deprecated = True
            elif t == "Soft_Dotted":
                d.soft_dotted = True
            elif t == "Logical_Order_Exception":
                d.logical_order_exception = True
            elif t == "Other_ID_Start":
                d.other_id_start = True
            elif t == "Other_ID_Continue":
                d.other_id_continue = True
            elif t == "Sentence_Terminal":
                d.sentence_terminal = True
            elif t == "Variation_Selector":
                d.variation_selector = True
            elif t == "Pattern_White_Space":
                d.pattern_white_space = True
            elif t == "Pattern_Syntax":
                d.pattern_syntax = True
            elif t == "Prepended_Concatenation_Mark":
                d.prepended_concatenation_mark = True
            elif t == "Regional_Indicator":
                d.regional_indicator = True
            else:
                raise RuntimeError("prop-list-data")


