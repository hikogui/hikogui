import ucd
import argparse

def parse_options():
    parser = argparse.ArgumentParser(description='Build c++ source files from Unicode ucd text files.')
    parser.add_argument("--compositions-output", dest="compositions_output_path", action="store", required=True)
    parser.add_argument("--compositions-template", dest="compositions_template_path", action="store", required=True)
    parser.add_argument("--decompositions-output", dest="decompositions_output_path", action="store", required=True)
    parser.add_argument("--decompositions-template", dest="decompositions_template_path", action="store", required=True)
    parser.add_argument("--index-output", dest="index_output_path", action="store", required=True)
    parser.add_argument("--index-template", dest="index_template_path", action="store", required=True)
    parser.add_argument("--descriptions-output", dest="descriptions_output_path", action="store", required=True)
    parser.add_argument("--descriptions-template", dest="descriptions_template_path", action="store", required=True)
    parser.add_argument("--normalize-output", dest="normalize_output_path", action="store", required=True)
    parser.add_argument("--normalize-template", dest="normalize_template_path", action="store", required=True)

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

def make_canonical_combining_class_table(chunks):
    canonical_combining_classes = []
    for chunk in chunks:
        for d in chunk.descriptions:
            canonical_combining_classes.append(d.canonical_combining_class)

    return canonical_combining_classes

def make_decomposition_bytes(chunks):
    decomposition_code_points = []
    decomposition_tuples = []

    max_length = 0
    max_index = 0
    for chunk in chunks:
        for d in chunk.descriptions:
            if len(d.decomposition_mapping) == 0:
                decomposition_tuples.append((0, 0, 0))

            else:
                index = len(decomposition_code_points)
                length = len(d.decomposition_mapping)
                type_ = d.decomposition_type_as_integer()
                decomposition_tuples.append((index, length, type_))

                decomposition_code_points += d.decomposition_mapping
                max_length = max(max_length, length)
                max_index = max(max_index, index)


    index_bit = max_index.bit_length()
    length_bit = max_length.bit_length()
    type_bit = 5
    tuple_bit = index_bit + length_bit + type_bit

    decomposition_tuples_as_int = [ (((index << length_bit) | length) << type_bit) | _type for index, length, _type in decomposition_tuples]

    decomposition_tuples_as_bytes = ucd.bits_as_bytes(decomposition_tuples_as_int, tuple_bit)
    decomposition_code_points_as_bytes = ucd.bits_as_bytes(decomposition_code_points, 21)
    return decomposition_tuples_as_bytes, decomposition_code_points_as_bytes, index_bit, length_bit, type_bit

def generate_normalize_output(template_path, output_path, chunks):
    ccc_table = make_canonical_combining_class_table(chunks)
    decomposition_bytes, decomposition_code_point_bytes, index_bit, length_bit, type_bit = make_decomposition_bytes(chunks)

    ucd.generate_output(
        template_path,
        output_path,
        ccc_table=ccc_table,
        decomposition_bytes=decomposition_bytes,
        decomposition_code_point_bytes=decomposition_code_point_bytes,
        index_bit=index_bit,
        length_bit=length_bit,
        type_bit=type_bit
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

    generate_normalize_output(options.normalize_template_path, options.normalize_output_path, chunks)

if __name__ == "__main__":
    main()
