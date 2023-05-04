
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys

def generate_decompositions(template_path, output_path, descriptions):
    print("Processing decomposition:", file=sys.stderr, flush=True)

    decomposition_type_enum = {"canonical": 0}
    code_points = []
    decomposition_tuples = []
    for x in descriptions:
        if len(x.decomposition_mapping) == 0:
            decomposition_tuples.append((0, 0, 0))

        else:
            cp_index = len(code_points)
            cp_size = len(x.decomposition_mapping)

            type_name = x.decomposition_type or "canonical"
            type_value = decomposition_type_enum.setdefault(type_name, len(decomposition_type_enum))
            decomposition_tuples.append((cp_index, cp_size, type_value))
            code_points += x.decomposition_mapping

    cp_index_width = max(x[0].bit_length() for x in decomposition_tuples)
    cp_size_width = max(x[1].bit_length() for x in decomposition_tuples)
    type_width = max(x[2].bit_length() for x in decomposition_tuples)

    decompositions = [(((x[0] << cp_size_width) | x[1]) << type_width) | x[2] for x in decomposition_tuples]

    decompositions, indices, chunk_size = deduplicate(decompositions)

    code_points_bytes, code_point_width = bits_as_bytes(code_points)
    indices_bytes, index_width = bits_as_bytes(indices)
    decompositions_bytes, decomposition_width = bits_as_bytes(decompositions)

    print("    chunk-size={} #indices={}:{} #decomposition={}:{} #code-points={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(decompositions), decomposition_width,
        len(code_points), code_point_width,
        len(indices_bytes) + len(decompositions_bytes) + len(code_points_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        chunk_size=chunk_size,
        cp_index_width=cp_index_width,
        cp_size_width=cp_size_width,
        type_width=type_width,
        indices_size=len(indices),
        indices_bytes=indices_bytes,
        index_width=index_width,
        decomposition_type_enum=decomposition_type_enum,
        decomposition_width=decomposition_width,
        decompositions_bytes=decompositions_bytes,
        code_points_bytes=code_points_bytes,
        code_point_width=code_point_width,
    )
