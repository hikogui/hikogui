
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_bidi_paired_bracket_types(template_path, output_path, descriptions):
    print("Processing bidi_paired_bracket_types:", file=sys.stderr, flush=True)

    bidi_paired_bracket_type_enum = {"n": 0}
    bidi_paired_bracket_types = []
    for x in descriptions:
        value = bidi_paired_bracket_type_enum.setdefault(
            x.bidi_paired_bracket_type, len(bidi_paired_bracket_type_enum))
        bidi_paired_bracket_types.append(value)

    bidi_paired_bracket_types, indices, chunk_size = deduplicate(bidi_paired_bracket_types)
    bidi_paired_bracket_types_bytes, bidi_paired_bracket_type_width = bits_as_bytes(bidi_paired_bracket_types)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #bidi_paired_bracket_types={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(bidi_paired_bracket_types), bidi_paired_bracket_type_width,
        len(indices_bytes) + len(bidi_paired_bracket_types_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        bidi_paired_bracket_type_enum=bidi_paired_bracket_type_enum,
        bidi_paired_bracket_type_width=bidi_paired_bracket_type_width,
        bidi_paired_bracket_types_bytes=bidi_paired_bracket_types_bytes
    )
