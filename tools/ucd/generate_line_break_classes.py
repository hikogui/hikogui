
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_line_break_classes(template_path, output_path, descriptions):
    print("Processing line_break_class:", file=sys.stderr, flush=True)

    line_break_class_enum = {"XX": 0}
    line_break_classes = []
    for x in descriptions:
        value = line_break_class_enum.setdefault(x.line_break, len(line_break_class_enum))
        line_break_classes.append(value)

    line_break_classes, indices, chunk_size = deduplicate(line_break_classes)
    line_break_classes_bytes, line_break_class_width = bits_as_bytes(line_break_classes)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #line_break_classes={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(line_break_classes), line_break_class_width,
        len(indices_bytes) + len(line_break_classes_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        line_break_class_enum=line_break_class_enum,
        line_break_class_width=line_break_class_width,
        line_break_classes_bytes=line_break_classes_bytes
    )
