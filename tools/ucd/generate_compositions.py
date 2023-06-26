
from .psp import psp_execute
from .deduplicate import deduplicate
from .bits_as_bytes import bits_as_bytes
import sys
import statistics

def generate_compositions(template_path, output_path, descriptions):
    print("Processing compositions:", file=sys.stderr, flush=True)
    compositions_info = {}
    for result_cp, d in enumerate(descriptions):
        if d.decomposition_type is not None or len(d.decomposition_mapping) != 2 or d.composition_exclusion:
            continue

        first_cp = d.decomposition_mapping[0]
        second_cp = d.decomposition_mapping[1]

        pairs = compositions_info.setdefault(first_cp, [])
        pairs.append((second_cp, result_cp))

    composition_tuples = [(0, 0)] * 0x110000
    code_points = []
    for first_cp, pairs in compositions_info.items():
        assert(len(pairs) > 0)
        composition_tuples[first_cp] = (len(code_points) // 2, len(pairs))
        pairs.sort()

        # First all the second_cp
        for pair in pairs:
            code_points.append(pair[0])

        # Next all the result_cp
        for pair in pairs:
            code_points.append(pair[1])

    mean_pairs = statistics.mean(x[1] for x in composition_tuples if x[1] > 0)
    print("    Mean number of compositions for a start code-point: {}".format(mean_pairs), file=sys.stderr)

    cp_index_width = max(x[0].bit_length() for x in composition_tuples)
    cp_size_width = max(x[1].bit_length() for x in composition_tuples)
    compositions = [(x[0] << cp_size_width) | x[1] for x in composition_tuples]

    compositions, indices, chunk_size = deduplicate(compositions)

    code_points_bytes, code_point_width = bits_as_bytes(code_points)
    indices_bytes, index_width = bits_as_bytes(indices)
    compositions_bytes, composition_width = bits_as_bytes(compositions)

    print("    chunk-size={} #indices={}:{} #composition={}:{} #code-points={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(compositions), composition_width,
        len(code_points), code_point_width,
        len(indices_bytes) + len(compositions_bytes) + len(code_points_bytes)),
        file=sys.stderr)

    psp_execute(
        template_path,
        output_path,
        cp_index_width=cp_index_width,
        cp_size_width=cp_size_width,
        chunk_size=chunk_size,
        indices_size=len(indices),
        indices_bytes=indices_bytes,
        index_width=index_width,
        compositions_bytes=compositions_bytes,
        composition_width=composition_width,
        code_points_bytes=code_points_bytes,
        code_point_width=code_point_width,
    )
