
import parse_ucd
import description

def parse_line_break(filename):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        line_break = columns[1]

        for code_point in code_points:
            d = description.descriptions[code_point]
            d.line_break = line_break


