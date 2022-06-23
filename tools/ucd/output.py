
import sys
from .psp import psp

def generate_output(filename, **args):
    print("Generating output using template {}".format(filename), file=sys.stderr, flush=True)
    text = open(filename, "r", encoding="utf-8").read()
    return psp(text, args, separator="$")
