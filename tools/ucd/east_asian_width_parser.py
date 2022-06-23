
import parse_ucd
import description

def parse_east_asian_width(filename):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        east_asian_width = columns[1]

        for code_point in code_points:
            d = description.descriptions[code_point]
            d.east_asian_width = east_asian_width


