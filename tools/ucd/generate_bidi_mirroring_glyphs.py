
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_bidi_mirroring_glyphs(template_path, output_path, descriptions):
    print("Processing bidi_mirroring_glyphs:", file=sys.stderr, flush=True)
    bidi_mirroring_glyphs = [x.bidi_mirroring_glyph or 0 for x in descriptions]

    bidi_mirroring_glyphs, indices, chunk_size = deduplicate(bidi_mirroring_glyphs)
    bidi_mirroring_glyphs_bytes, bidi_mirroring_glyph_width = bits_as_bytes(bidi_mirroring_glyphs)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #bidi_mirroring_glyphs={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(bidi_mirroring_glyphs), bidi_mirroring_glyph_width,
        len(indices_bytes) + len(bidi_mirroring_glyphs_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        bidi_mirroring_glyph_width=bidi_mirroring_glyph_width,
        bidi_mirroring_glyphs_bytes=bidi_mirroring_glyphs_bytes
    )
