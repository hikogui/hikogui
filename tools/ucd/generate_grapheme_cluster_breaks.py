
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys


def generate_grapheme_cluster_breaks(template_path, output_path, descriptions):
    print("Processing grapheme_cluster_break:", file=sys.stderr, flush=True)

    grapheme_cluster_break_enum = {"Other": 0}
    grapheme_cluster_breaks = []
    for x in descriptions:
        name = "Extended_Pictographic" if x.extended_pictographic else x.grapheme_cluster_break
        value = grapheme_cluster_break_enum.setdefault(name, len(grapheme_cluster_break_enum))
        grapheme_cluster_breaks.append(value)

    grapheme_cluster_breaks, indices, chunk_size = deduplicate(grapheme_cluster_breaks)
    grapheme_cluster_breaks_bytes, grapheme_cluster_break_width = bits_as_bytes(grapheme_cluster_breaks)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #grapheme_cluster_breaks={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(grapheme_cluster_breaks), grapheme_cluster_break_width,
        len(indices_bytes) + len(grapheme_cluster_breaks_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        grapheme_cluster_break_enum=grapheme_cluster_break_enum,
        grapheme_cluster_break_width=grapheme_cluster_break_width,
        grapheme_cluster_breaks_bytes=grapheme_cluster_breaks_bytes
    )
