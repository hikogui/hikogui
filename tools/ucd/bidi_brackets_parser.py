
from .ucd_parser import parse_ucd

def parse_bidi_brackets(filename, descriptions):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        bidi_paired_bracket = int(columns[1], 16)
        bidi_paired_bracket_type = columns[2]

        for code_point in code_points:
            d = descriptions[code_point]
            d.bidi_paired_bracket_type = bidi_paired_bracket_type
            d.bidi_paired_bracket = bidi_paired_bracket


