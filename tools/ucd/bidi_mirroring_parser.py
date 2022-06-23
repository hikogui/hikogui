
import parse_ucd
import description

def parse_bidi_mirroring(filename):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        bidi_mirroring_glyph = int(columns[1], 16)

        for code_point in code_points:
            d = description.descriptions[code_point]
            d.bidi_mirroring_glyph = bidi_mirroring_glyph


