
import sys
from .psp import psp

def generate_output(template_path, output_path, **args):
    print("Generating output {} using template {}".format(output_path, template_path), file=sys.stderr, flush=True)
    template_text = open(template_path, "r", encoding="utf-8").read()
    output_text = psp(template_text, args, separator="$")
    open(output_path, "w", encoding="utf-8").write(output_text)
