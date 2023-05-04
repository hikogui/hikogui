
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_east_asian_widths(template_path, output_path, descriptions):
    print("Processing east_asian_widths:", file=sys.stderr, flush=True)

    east_asian_width_enum = {"N": 0}
    east_asian_widths = []
    for x in descriptions:
        value = east_asian_width_enum.setdefault(x.east_asian_width, len(east_asian_width_enum))
        east_asian_widths.append(value)

    east_asian_widths, indices, chunk_size = deduplicate(east_asian_widths)
    east_asian_widths_bytes, east_asian_width_width = bits_as_bytes(east_asian_widths)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #east_asian_widths={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(east_asian_widths), east_asian_width_width,
        len(indices_bytes) + len(east_asian_widths_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        east_asian_width_enum=east_asian_width_enum,
        east_asian_width_width=east_asian_width_width,
        east_asian_widths_bytes=east_asian_widths_bytes
    )
