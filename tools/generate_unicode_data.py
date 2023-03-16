import ucd
import argparse
import sys

def parse_options():
    parser = argparse.ArgumentParser(description='Build c++ source files from Unicode ucd text files.')
    parser.add_argument("--compositions-output", dest="compositions_output_path", action="store", required=True)
    parser.add_argument("--compositions-template", dest="compositions_template_path", action="store", required=True)
    parser.add_argument("--index-output", dest="index_output_path", action="store", required=True)
    parser.add_argument("--index-template", dest="index_template_path", action="store", required=True)
    parser.add_argument("--descriptions-output", dest="descriptions_output_path", action="store", required=True)
    parser.add_argument("--descriptions-template", dest="descriptions_template_path", action="store", required=True)
    parser.add_argument("--canonical_combining_classes-output", dest="canonical_combining_classes_output_path", action="store", required=True)
    parser.add_argument("--canonical_combining_classes-template", dest="canonical_combining_classes_template_path", action="store", required=True)
    parser.add_argument("--decompositions-output", dest="decompositions_output_path", action="store", required=True)
    parser.add_argument("--decompositions-template", dest="decompositions_template_path", action="store", required=True)

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

    compositions = ucd.make_composition_table(descriptions)

    indices, chunks = ucd.deduplicate_chunks(descriptions)

    ucd.generate_output(options.compositions_template_path, options.compositions_output_path, compositions=compositions)
    ucd.generate_output(options.index_template_path, options.index_output_path, indices=indices)
    ucd.generate_output(options.descriptions_template_path, options.descriptions_output_path, chunks=chunks)

    generate_canonical_combining_class(options.canonical_combining_classes_template_path, options.canonical_combining_classes_output_path, descriptions)
    generate_decomposition(options.decompositions_template_path, options.decompositions_output_path, descriptions)


if __name__ == "__main__":
    main()
