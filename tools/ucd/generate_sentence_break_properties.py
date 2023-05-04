
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys


def generate_sentence_break_properties(template_path, output_path, descriptions):
    print("Processing sentence_break_property:", file=sys.stderr, flush=True)

    sentence_break_property_enum = {"Other": 0}
    sentence_break_properties = []
    for x in descriptions:
        value = sentence_break_property_enum.setdefault(x.sentence_break, len(sentence_break_property_enum))
        sentence_break_properties.append(value)

    sentence_break_properties, indices, chunk_size = deduplicate(sentence_break_properties)
    sentence_break_properties_bytes, sentence_break_property_width = bits_as_bytes(sentence_break_properties)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #sentence_break_properties={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(sentence_break_properties), sentence_break_property_width,
        len(indices_bytes) + len(sentence_break_properties_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        sentence_break_property_enum=sentence_break_property_enum,
        sentence_break_property_width=sentence_break_property_width,
        sentence_break_properties_bytes=sentence_break_properties_bytes
    )
