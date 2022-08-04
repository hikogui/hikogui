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
    decompositions = ucd.make_decomposition_table(descriptions)

    indices, chunks = ucd.deduplicate_chunks(descriptions)

    ucd.generate_output(options.compositions_template_path, options.compositions_output_path, compositions=compositions)
    ucd.generate_output(options.decompositions_template_path, options.decompositions_output_path, decompositions=decompositions)
    ucd.generate_output(options.index_template_path, options.index_output_path, indices=indices)
    ucd.generate_output(options.descriptions_template_path, options.descriptions_output_path, chunks=chunks)

if __name__ == "__main__":
    main()
