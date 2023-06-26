
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_lexical_classes(template_path, output_path, descriptions):
    print("Processing lexical_classes:", file=sys.stderr, flush=True)

    lexical_classes = []
    for x in descriptions:
        if x.pattern_syntax:
            lexical_classes.append("syntax")
        elif x.pattern_white_space:
            lexical_classes.append("white_space")
        elif x.other_id_start:
            lexical_classes.append("id_start")
        elif x.general_category in ("Lu", "Ll", "Lt", "Lm", "Lo", "Nl"):
            lexical_classes.append("id_start")
        elif x.other_id_continue:
            lexical_classes.append("id_continue")
        elif x.general_category in ("Mn", "Mc", "Nd", "Pc"):
            lexical_classes.append("id_continue")
        else:
            lexical_classes.append("other")

    # For performance improvement of the is_* functions the enum values should be
    # sorted by grouped by their first letter. "Cn" is used most, move it to first person
    # so that the table will mostly contain zeros.
    lexical_class_enum_names = sorted(set(lexical_classes))
    lexical_class_enum_names.remove("other")
    lexical_class_enum_names.insert(0, "other")

    lexical_class_enum = {}
    for i, name in enumerate(lexical_class_enum_names):
        lexical_class_enum[name] = i

    lexical_classes = [lexical_class_enum[x] for x in lexical_classes]

    lexical_classes, indices, chunk_size = deduplicate(lexical_classes)
    lexical_classes_bytes, lexical_class_width = bits_as_bytes(lexical_classes)
    indices_bytes, index_width = bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #lexical_classes={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(lexical_classes), lexical_class_width,
        len(indices_bytes) + len(lexical_classes_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        lexical_class_enum=lexical_class_enum,
        lexical_class_width=lexical_class_width,
        lexical_classes_bytes=lexical_classes_bytes
    )
