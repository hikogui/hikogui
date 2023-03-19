import ucd
import argparse
import sys
import statistics

def parse_options():
    parser = argparse.ArgumentParser(description='Build c++ source files from Unicode ucd text files.')
    parser.add_argument("--index-output", dest="index_output_path", action="store", required=True)
    parser.add_argument("--index-template", dest="index_template_path", action="store", required=True)
    parser.add_argument("--descriptions-output", dest="descriptions_output_path", action="store", required=True)
    parser.add_argument("--descriptions-template", dest="descriptions_template_path", action="store", required=True)
    parser.add_argument("--canonical_combining_classes-output", dest="canonical_combining_classes_output_path", action="store", required=True)
    parser.add_argument("--canonical_combining_classes-template", dest="canonical_combining_classes_template_path", action="store", required=True)
    parser.add_argument("--decompositions-output", dest="decompositions_output_path", action="store", required=True)
    parser.add_argument("--decompositions-template", dest="decompositions_template_path", action="store", required=True)
    parser.add_argument("--compositions-output", dest="compositions_output_path", action="store", required=True)
    parser.add_argument("--compositions-template", dest="compositions_template_path", action="store", required=True)

    parser.add_argument("--bidi-brackets", dest="bidi_brackets_path", action="store", required=True)
    parser.add_argument("--bidi-mirroring", dest="bidi_mirroring_path", action="store", required=True)
    parser.add_argument("--composition-exclusions", dest="composition_exclusions_path", action="store", required=True)
    parser.add_argument("--east-asian-width", dest="east_asian_width_path", action="store", required=True)
    parser.add_argument("--emoji-data", dest="emoji_data_path", action="store", required=True)
    parser.add_argument("--grapheme-break-property", dest="grapheme_break_property_path", action="store", required=True)
    parser.add_argument("--line-break", dest="line_break_path", action="store", required=True)
    parser.add_argument("--scripts", dest="scripts_path", action="store", required=True)
    parser.add_argument("--sentence-break-property", dest="sentence_break_property_path", action="store", required=True)
    parser.add_argument("--unicode-data", dest="unicode_data_path", action="store", required=True)
    parser.add_argument("--word-break-property", dest="word_break_property_path", action="store", required=True)
    return parser.parse_args()




def generate_canonical_combining_class(template_path, output_path, descriptions):
    print("Processing canonical-combining-class:", file=sys.stderr, flush=True)
    canonical_combining_classes = [x.canonical_combining_class for x in descriptions]

    canonical_combining_classes, indices, chunk_size = ucd.deduplicate(canonical_combining_classes)
    canonical_combining_classes_bytes, canonical_combining_class_width = ucd.bits_as_bytes(canonical_combining_classes)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #ccc={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(canonical_combining_classes), canonical_combining_class_width,
        len(indices_bytes) + len(canonical_combining_classes_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        canonical_combining_class_width=canonical_combining_class_width,
        canonical_combining_classes_bytes=canonical_combining_classes_bytes
    )

def generate_decomposition(template_path, output_path, descriptions):
    print("Processing decomposition:", file=sys.stderr, flush=True)

    code_points = []
    decomposition_tuples = []
    for x in descriptions:
        if len(x.decomposition_mapping) == 0:
            decomposition_tuples.append((0, 0, 0))

        else:
            cp_index = len(code_points)
            cp_size = len(x.decomposition_mapping)
            type_ = x.decomposition_type_as_integer()
            decomposition_tuples.append((cp_index, cp_size, type_))
            code_points += x.decomposition_mapping

    cp_index_width = max(x[0].bit_length() for x in decomposition_tuples)
    cp_size_width = max(x[1].bit_length() for x in decomposition_tuples)
    type_width = max(x[2].bit_length() for x in decomposition_tuples)

    decompositions = [(((x[0] << cp_size_width) | x[1]) << type_width) | x[2] for x in decomposition_tuples]

    decompositions, indices, chunk_size = ucd.deduplicate(decompositions)

    code_points_bytes, code_point_width = ucd.bits_as_bytes(code_points)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)
    decompositions_bytes, decomposition_width = ucd.bits_as_bytes(decompositions)

    print("    chunk-size={} #indices={}:{} #decomposition={}:{} #code-points={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(decompositions), decomposition_width,
        len(code_points), code_point_width,
        len(indices_bytes) + len(decompositions_bytes) + len(code_points_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        cp_index_width=cp_index_width,
        cp_size_width=cp_size_width,
        type_width=type_width,
        indices_size=len(indices),
        indices_bytes=indices_bytes,
        index_width=index_width,
        decompositions_bytes=decompositions_bytes,
        decomposition_width=decomposition_width,
        code_points_bytes=code_points_bytes,
        code_point_width=code_point_width,
    )

def generate_composition(template_path, output_path, descriptions):
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

    compositions, indices, chunk_size = ucd.deduplicate(compositions)

    code_points_bytes, code_point_width = ucd.bits_as_bytes(code_points)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)
    compositions_bytes, composition_width = ucd.bits_as_bytes(compositions)

    print("    chunk-size={} #indices={}:{} #composition={}:{} #code-points={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(compositions), composition_width,
        len(code_points), code_point_width,
        len(indices_bytes) + len(compositions_bytes) + len(code_points_bytes)),
        file=sys.stderr)

    ucd.generate_output(
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


def main():
    options = parse_options()

    descriptions = ucd.initialize_descriptions()
    ucd.parse_bidi_brackets(options.bidi_brackets_path, descriptions)
    ucd.parse_bidi_mirroring(options.bidi_mirroring_path, descriptions)
    ucd.parse_composition_exclusions(options.composition_exclusions_path, descriptions)
    ucd.parse_east_asian_width(options.east_asian_width_path, descriptions)
    ucd.parse_emoji_data(options.emoji_data_path, descriptions)
    ucd.parse_grapheme_break_property(options.grapheme_break_property_path, descriptions)
    ucd.parse_line_break(options.line_break_path, descriptions)
    ucd.parse_scripts(options.scripts_path, descriptions)
    ucd.parse_sentence_break_property(options.sentence_break_property_path, descriptions)
    ucd.parse_unicode_data(options.unicode_data_path, descriptions)
    ucd.parse_word_break_property(options.word_break_property_path, descriptions)
    ucd.add_hangul_decompositions(descriptions)

    indices, chunks = ucd.deduplicate_chunks(descriptions)

    ucd.generate_output(options.index_template_path, options.index_output_path, indices=indices)
    ucd.generate_output(options.descriptions_template_path, options.descriptions_output_path, chunks=chunks)

    generate_canonical_combining_class(options.canonical_combining_classes_template_path, options.canonical_combining_classes_output_path, descriptions)
    generate_decomposition(options.decompositions_template_path, options.decompositions_output_path, descriptions)
    generate_composition(options.compositions_template_path, options.compositions_output_path, descriptions)


if __name__ == "__main__":
    main()
