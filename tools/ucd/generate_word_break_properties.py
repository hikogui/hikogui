
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_word_break_properties(template_path, output_path, descriptions):
    print("Processing word_break_property:", file=sys.stderr, flush=True)

    word_break_property_enum = {"Other": 0}
    word_break_properties = []
    for x in descriptions:
        value = word_break_property_enum.setdefault(x.word_break, len(word_break_property_enum))
        word_break_properties.append(value)

    word_break_properties, indices, chunk_size = deduplicate(word_break_properties)
    word_break_properties_bytes, word_break_property_width = bits_as_bytes(word_break_properties)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #word_break_properties={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(word_break_properties), word_break_property_width,
        len(indices_bytes) + len(word_break_properties_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        word_break_property_enum=word_break_property_enum,
        word_break_property_width=word_break_property_width,
        word_break_properties_bytes=word_break_properties_bytes
    )
