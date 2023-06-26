
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys


def generate_canonical_combining_classes(template_path, output_path, descriptions):
    print("Processing canonical_combining_class:", file=sys.stderr, flush=True)
    canonical_combining_classes = [x.canonical_combining_class for x in descriptions]

    canonical_combining_classes, indices, chunk_size = deduplicate(canonical_combining_classes)
    canonical_combining_classes_bytes, canonical_combining_class_width = bits_as_bytes(canonical_combining_classes)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #canonical_combining_classes={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(canonical_combining_classes), canonical_combining_class_width,
        len(indices_bytes) + len(canonical_combining_classes_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        canonical_combining_class_width=canonical_combining_class_width,
        canonical_combining_classes_bytes=canonical_combining_classes_bytes
    )
