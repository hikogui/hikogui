

import sys

def invert_bracket(c):
    i = "(){}[]".find(c)
    return ")(}{]["[i]

def bracket_check(stack, c, filename, line_nr):
    if c in "({[":
        stack.append((invert_bracket(c), line_nr))

    elif c in ")}]":
        if stack:
            if c == stack[-1][0]:
                stack.pop()
            else:
                raise RuntimeError("{}:{}: Close bracket '{}' mismatch with '{}' on line {}.".format(filename, line_nr, c, invert_bracket(stack[-1][0]), stack[-1][1])) 
        else:
            raise RuntimeError("{}:{}: Close bracket '{}' without open bracket.".format(filename, line_nr, c))

def check(filename, text):
    stack = []
    in_string = None
    escape = False
    line_nr = 1

    # None - Idle
    # "/"  - Found "/"
    # "L"  - Found "//"
    # "B"  - Found "/*"
    # "b"  - Found "/* .. *"
    # "\'" - Found "\'"
    # "\"" - Found "\""
    state = None

    prev_c = None
    for c in text:
        if c == "\n":
            line_nr += 1

        if escape:
            escape = False
            continue
        elif c == "\\":
            escape = True
            continue

        if state is None:
            if c == "'" and prev_c not in "0123456789abcdefABCDEF":
                state = "'"
            elif c == "\"":
                state = "\""
            elif c == "/":
                state = "/"
            else:
                bracket_check(stack, c, filename, line_nr)

        elif state == "/":
            if c == "/":
                state = "L"
            elif c == "*":
                state = "B"
            else:
                bracket_check(stack, c, filename, line_nr)
                state = None

        elif state == "'":
            if c == "'":
                state = None

        elif state == "\"":
            if c == "\"":
                state = None

        elif state == "L":
            if c == "\n":
                state = None

        elif state == "B":
            if c == "*":
                state = "b"

        elif state == "b":
            if c == "/":
                state = None
            else:
                state = "B"

        prev_c = c

    if stack:
        raise RuntimeError("{}:{}: Missing close bracket matching {} on line {}.".format(filename, line_nr, invert_bracket(stack[-1][0]), stack[-1][1]))

def main(filenames):
    for filename in filenames:
        with open(filename, "r") as fd:
            try:
                check(filename, fd.read())
            except Exception as e:
                print(e, file=sys.stderr)

if __name__ == "__main__":
    main(sys.argv[1:])

