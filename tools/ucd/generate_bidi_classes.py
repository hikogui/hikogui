
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_bidi_classes(template_path, output_path, descriptions):
    print("Processing bidi_classes:", file=sys.stderr, flush=True)

    bidi_class_enum = {"ON": 0, "L": 1, "R": 2}
    bidi_classes = []
    for x in descriptions:
        value = bidi_class_enum.setdefault(x.bidi_class, len(bidi_class_enum))
        bidi_classes.append(value)

    bidi_classes, indices, chunk_size = deduplicate(bidi_classes)
    bidi_classes_bytes, bidi_class_width = bits_as_bytes(bidi_classes)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #bidi_classes={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(bidi_classes), bidi_class_width,
        len(indices_bytes) + len(bidi_classes_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        bidi_class_enum=bidi_class_enum,
        bidi_class_width=bidi_class_width,
        bidi_classes_bytes=bidi_classes_bytes
    )
