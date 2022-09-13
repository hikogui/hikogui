
import sys

def make_composition_table(descriptions):
    """Extract composition table from the descriptions.

    Compositions are always canonical and are always from 2
    code-points to 1 code-point.

    Since the descriptor table can be accessed at O(1) we can resolve
    the first code-point and point into the composition table for a
    set of code-point pairs to complete the composition.

    The composition table contains pairs of code-points:
     - second decomposed code-point.
     - result composed code-point.

    Code-points are only 21-bits. We can store 3 of them in a 64 bit integer (with an extra bit).

      ---------:| ---------------------:
       1 pair   | <1><ffff><res0><sec0>
       2 pair   | <0><sec1><res0><sec0>
                | <1><ffff><ffff><res1>
       3 pair   | <0><sec1><res0><sec0>
                | <1><res2><sec2><res1>
       4 pair   | <0><sec1><res0><sec0>
                | <0><res2><sec2><res1>
                | <1><ffff><res3><sec3>

    @param[in,out] descriptions A table of unicode descriptions
    @return The table of compositions, a list of 64 bit integers.
    """

    print("Creating composition table.", file=sys.stderr, flush=True)

    compositions = {}
    for result_cp, d in enumerate(descriptions):
        if d.decomposition_type != "canonical" or len(d.decomposition_mapping) != 2 or d.composition_exclusion:
            continue

        first_cp = d.decomposition_mapping[0]
        second_cp = d.decomposition_mapping[1]

        mappings = compositions.setdefault(first_cp, [])
        mappings.append((second_cp, result_cp))

    keys = list(compositions.keys())
    keys.sort()

    r = []
    for first_cp in keys:
        mappings = compositions[first_cp]
        mappings.sort()

        d = descriptions[first_cp]
        d.g_composition_index = len(r)

        for i, (second_cp, result_cp) in enumerate(mappings):
            last = i == len(mappings) - 1

            if i % 3 == 0:
                value = result_cp << 21 | second_cp
                if last:
                    r.append(value | 0xfffffc0000000000)

            elif i % 3 == 1:
                value |= second_cp << 42
                r.append(value)
                value = result_cp
                if last:
                    r.append(value | 0xffffffffffe00000) 

            else:
                value |= second_cp << 21
                value |= result_cp << 42
                if last:
                    value |= 0x8000000000000000
                r.append(value)

    print("Composition table has {} entries.".format(len(r)), file=sys.stderr, flush=True)
    return r

