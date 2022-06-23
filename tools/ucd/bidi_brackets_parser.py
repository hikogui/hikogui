
import parse_ucd
import description

def parse_bidi_brackets(filename):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        bidi_paired_bracket_type = columns[1]
        bidi_paired_bracket = int(columns[2], 16)

        for code_point in code_points:
            d = description.descriptions[code_point]
            d.bidi_paired_bracket_type = bidi_paired_bracket_type
            d.bidi_paired_bracket = bidi_paired_bracket


