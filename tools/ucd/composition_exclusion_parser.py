
import parse_ucd
import description

def parse_composition_exclusion(filename):
    for columns in parse_ucd(filename):
        code_points = columns[0]

        for code_point in code_points:
            d = description.descriptions[code_point]
            d.composition_exclusion = True


