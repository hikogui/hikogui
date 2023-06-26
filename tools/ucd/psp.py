
import sys

def get_expression(text, begin, separator):
    """Find the end of a expression or statement
    An expression or statement ends at a new-line or at the separator,
    unless the new-line or separator is encountered inside a string-literal or
    inside matching bracket pairs.

    @param text The total text being parsed.
    @param begin The index at the start of the expression or statement.
    @param separator The character which ends an expression (not statement).
    @return (index beyond the terminator, the expression, The terminator or empty on end-of-text).
    """
    end_chars = separator + "\n\r\f"
    bracket_stack = 0
    in_string = None

    end = len(text)
    i = begin
    while i < end:
        c = text[i]
        if in_string:
            if c == in_string:
                in_string = None
            elif c == "\\":
                i += 1
        elif c in "'\"":
            in_string = c
        elif c in "[({":
            bracket_stack += 1
        elif c in "])}":
            bracket_stack -= 1
        elif c in end_chars and not bracket_stack:
            break

        i += 1

    expression = text[begin:i].strip()
    terminator = text[i:i + 1]
    return i + 1, expression, terminator

def I(stack, adjust=0):
    return "    " * (len(stack) + adjust)

def append_text(program, stack, text, i, j):
    for line in text[i:j].splitlines(True):
        if line.endswith("\\\n"):
            # Strip the linefeed if it is escaped.
            line = line[:-2]

        if line.isspace():
            # If the line is only white-space, it may get removed if there is a python statement following it.
            program.append(('w', I(stack) + '__r.append({})'.format(repr(line))))

        else:
            program.append(('t', I(stack) + '__r.append({})'.format(repr(line))))
    return j

def append_expression(program, stack, expression):
    program.append(('e', I(stack) + "__r.append(str({}))".format(expression)))

def remove_whitespace_before_statement(program):
    if program[-1][0] == 'w':
        program.pop()

def append_statement(program, stack, adjust, statement):
    remove_whitespace_before_statement(program)
    program.append(('s', I(stack, adjust) + statement))

def merge_program(program):
    return "\n".join(x[1] for x in program)

def find(text, separator, i):
    j = text.find(separator, i)
    return j if j != -1 else len(text)

def parse_psp(text, separator):
    """Parse PSP text and return a python script

    @param text The text to parse.
    @param separator The separator to use to detect Python statements and expressions
    @return A python program.
    """
    program = [('i', '__r = []')]
    stack = []
    i = 0

    while True:
        j = find(text, separator, i)
        i = append_text(program, stack, text, i, j)
        if i == len(text):
            break

        i, expression, terminator = get_expression(text, i + 1, separator)

        if terminator == separator and expression == "":
            # Separator escape, print the whole string.
            append_expression(program, stack, repr(separator))

        elif terminator == separator:
            # Print the result of the expression.
            append_expression(program, stack, expression)

        elif expression.startswith("end"):
            # End a block.
            remove_whitespace_before_statement(program)
            if not stack:
                raise RuntimeError("%end found when no open python blocks")
            stack.pop()

        elif expression.startswith("elif ") or expression.startswith("else:"):
            # elif and else statements both end and start a block.
            append_statement(program, stack, -1, expression)

        elif expression.endswith(":"):
            # statements ending with : start a new block.
            append_statement(program, stack, 0, expression)
            stack.append(expression)

        else:
            # Other statement are in the current block.
            append_statement(program, stack, 0, expression)
    
    if stack:
        raise RuntimeError("Missing %end block stack={}".format(stack))

    return merge_program(program)

def psp(text, namespace, separator="%"):
    """A very simple template language parser and evaluate.

    Python expression
    -----------------
    A python expression is enclosed between two separators '%'.

    Python statement
    ----------------
    A python statement follows a separator '%'.

    Python statements do not have to follow any kind of indentation,
    instead the current block is ended with the following statements:
     - %else:
     - %elif expression:
     - %end

    Separator escape
    ----------------
    Two separators '%' with nothing or only whitespace between them
    is replaced by a single separator.

    @param text The text to parse and execute
    @param namespace A dictionary passed when evaluating the template
    @param separator The separator to use to detect python code.
    @return The result of the evaluated text.

    Text
    ----
    The rest is text.
    """

    python_code = parse_psp(text, separator)
    exec(python_code, globals(), namespace)
    return "".join(namespace["__r"])

def psp_execute(template_path, output_path, **args):
    print("    Generating output {} using template {}".format(output_path, template_path), file=sys.stderr, flush=True)
    template_text = open(template_path, "r", encoding="utf-8").read()
    output_text = psp(template_text, args, separator="$")
    open(output_path, "w", encoding="utf-8").write(output_text)
