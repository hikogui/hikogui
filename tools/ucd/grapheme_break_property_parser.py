
from .ucd_parser import parse_ucd

def parse_grapheme_break_property(filename, descriptions):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        grapheme_cluster_break = columns[1]

        for code_point in code_points:
            d = descriptions[code_point]
            d.grapheme_cluster_break = grapheme_cluster_break


