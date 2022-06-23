
from .ucd_parser import parse_ucd

def parse_composition_exclusions(filename, descriptions):
    for columns in parse_ucd(filename):
        code_points = columns[0]

        for code_point in code_points:
            d = descriptions[code_point]
            d.composition_exclusion = True


