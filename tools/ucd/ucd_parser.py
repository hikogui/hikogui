
import sys

def strip_comment(line):
    i = line.find("#")
    if i >= 0:
        return line[:i]
    else:
        return line

def parse_code_point_range(code_point):
    columns = code_point.split("..")
    s = int(columns[0], 16)
    if len(columns) == 1:
        e = s
    elif len(columns) == 2:
        e = int(columns[1], 16)
    else:
        raise RuntimeError("code-point-range")

    return range(s, e + 1)

def parse_ucd(filename):
    """Generically parse any of the Unicode Data text files.
    """
    print("Parsing '{}'.".format(filename), file=sys.stderr, flush=True)
    with open(filename, "r", encoding="utf-8") as fd:
        for line in fd:
            line = strip_comment(line)
            columns = [x.strip() for x in line.split(";")]
            if not columns[0]:
                continue

            # First columns is a unicode or range of unicode.
            columns[0] = parse_code_point_range(columns[0])
            yield columns

