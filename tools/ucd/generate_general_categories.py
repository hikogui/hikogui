
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_general_categories(template_path, output_path, descriptions):
    print("Processing general_categories:", file=sys.stderr, flush=True)

    # For performance improvement of the is_* functions the enum values should be
    # sorted by grouped by their first letter. "Cn" is used most, move it to first person
    # so that the table will mostly contain zeros.
    general_category_enum_names = sorted(set(x.general_category for x in descriptions))
    general_category_enum_names.remove("Cn")
    general_category_enum_names.insert(0, "Cn")

    general_category_enum = {}
    for i, name in enumerate(general_category_enum_names):
        general_category_enum[name] = i

    general_categories = [general_category_enum[x.general_category] for x in descriptions]

    general_categories, indices, chunk_size = deduplicate(general_categories)
    general_categories_bytes, general_category_width = bits_as_bytes(general_categories)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #general_categories={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(general_categories), general_category_width,
        len(indices_bytes) + len(general_categories_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        general_category_enum=general_category_enum,
        general_category_width=general_category_width,
        general_categories_bytes=general_categories_bytes
    )
