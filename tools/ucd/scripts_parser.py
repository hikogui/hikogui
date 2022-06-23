
import parse_ucd
import description

def parse_scripts(filename):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        script = columns[1]

        for code_point in code_points:
            d = description.descriptions[code_point]
            d.script = script


