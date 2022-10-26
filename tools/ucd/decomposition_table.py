
import sys


def make_decomposition_table(descriptions):
    """Extract decomposition table from the descriptions.

    If the decomposition is a single code-point than we store that code-point directly in the
    description table, otherwise the description table has an index into the decomposition table.

    The index is encoded as a number between 0x11'0000 - 0x1f'fffe, this uses the invalid range of
    unicode code points that still fits in 21 bits. The value 0x1f'ffff means there is no decomposition.


    Code-points are only 21-bits. We can store 3 of them in a 64 bit integer (with an extra bit).

      --------------:| ---------------------:
       1 code-point | <1><ffff><ffff><cpt0>
       2 code-points| <1><ffff><cpt1><cpt0>
       3 code-points| <1><cpt2><cpt1><cpt0>
       4 code-points| <0><cpt2><cpt1><cpt0>
                    | <1><ffff><ffff><cpt3>
       5 code-points| <0><cpt2><cpt1><cpt0>
                    | <1><ffff><cpt4><cpt3>
       6 code-points| <0><cpt2><cpt1><cpt0>
                    | <1><cpt5><cpt4><cpt3>

    @param[in,out] descriptions A table of unicode descriptions
    @return The table of decompositions, a list of 64 bit integers.
    """

    print("Creating decomposition table.", file=sys.stderr, flush=True)

    r = []
    for d in descriptions:
        if len(d.decomposition_mapping) == 0:
            d.g_decomposition_index = 0x1fffff
        elif len(d.decomposition_mapping) == 1:
            d.g_decomposition_index = d.decomposition_mapping[0]
        else:
            d.g_decomposition_index = len(r) + 0x110000

            for i, code_point in enumerate(d.decomposition_mapping):
                last = i == len(d.decomposition_mapping) - 1

                if i % 3 == 0:
                    value = code_point
                    if last:
                        r.append(value | 0xffffffffffe00000) 

                        r.append(value | 0xfffffc0000000000)

                elif i % 3 == 1:
                    value |= code_point << 21
                    if last:
                        r.append(value | 0xfffffc0000000000)

                else:
                    value |= code_point << 42
                    if last:
                        value |= 0x8000000000000000
                    r.append(value)

    print("Decomposition table has {} entries.".format(len(r)), file=sys.stderr, flush=True)
    return r
