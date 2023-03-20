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

    parser.add_argument("--general-categories-output", dest="general_categories_output_path", action="store", required=True)
    parser.add_argument("--general-categories-template", dest="general_categories_template_path", action="store", required=True)
    parser.add_argument("--scripts-output", dest="scripts_output_path", action="store", required=True)
    parser.add_argument("--scripts-template", dest="scripts_template_path", action="store", required=True)
    parser.add_argument("--bidi-classes-output", dest="bidi_classes_output_path", action="store", required=True)
    parser.add_argument("--bidi-classes-template", dest="bidi_classes_template_path", action="store", required=True)
    parser.add_argument("--bidi-paired-bracket-types-output", dest="bidi_paired_bracket_types_output_path", action="store", required=True)
    parser.add_argument("--bidi-paired-bracket-types-template", dest="bidi_paired_bracket_types_template_path", action="store", required=True)
    parser.add_argument("--bidi-mirroring-glyphs-output", dest="bidi_mirroring_glyphs_output_path", action="store", required=True)
    parser.add_argument("--bidi-mirroring-glyphs-template", dest="bidi_mirroring_glyphs_template_path", action="store", required=True)
    parser.add_argument("--east-asian-widths-output", dest="east_asian_widths_output_path", action="store", required=True)
    parser.add_argument("--east-asian-widths-template", dest="east_asian_widths_template_path", action="store", required=True)
    parser.add_argument("--grapheme-cluster-breaks-output", dest="grapheme_cluster_breaks_output_path", action="store", required=True)
    parser.add_argument("--grapheme-cluster-breaks-template", dest="grapheme_cluster_breaks_template_path", action="store", required=True)
    parser.add_argument("--line-break-classes-output", dest="line_break_classes_output_path", action="store", required=True)
    parser.add_argument("--line-break-classes-template", dest="line_break_classes_template_path", action="store", required=True)
    parser.add_argument("--word-break-properties-output", dest="word_break_properties_output_path", action="store", required=True)
    parser.add_argument("--word-break-properties-template", dest="word_break_properties_template_path", action="store", required=True)
    parser.add_argument("--sentence-break-properties-output", dest="sentence_break_properties_output_path", action="store", required=True)
    parser.add_argument("--sentence-break-properties-template", dest="sentence_break_properties_template_path", action="store", required=True)
    parser.add_argument("--canonical-combining-classes-output", dest="canonical_combining_classes_output_path", action="store", required=True)
    parser.add_argument("--canonical-combining-classes-template", dest="canonical_combining_classes_template_path", action="store", required=True)
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
    parser.add_argument("--line-break", dest="line_break_class_path", action="store", required=True)
    parser.add_argument("--scripts", dest="scripts_path", action="store", required=True)
    parser.add_argument("--sentence-break-property", dest="sentence_break_property_path", action="store", required=True)
    parser.add_argument("--unicode-data", dest="unicode_data_path", action="store", required=True)
    parser.add_argument("--word-break-property", dest="word_break_property_path", action="store", required=True)
    return parser.parse_args()


def generate_general_categories(template_path, output_path, descriptions):
    print("Processing general_categories:", file=sys.stderr, flush=True)
    general_categories = [x.general_category_as_integer() for x in descriptions]

    general_categories, indices, chunk_size = ucd.deduplicate(general_categories)
    general_categories_bytes, general_category_width = ucd.bits_as_bytes(general_categories)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #general_categories={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(general_categories), general_category_width,
        len(indices_bytes) + len(general_categories_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        general_category_width=general_category_width,
        general_categories_bytes=general_categories_bytes
    )

def generate_scripts(template_path, output_path, descriptions):
    print("Processing scripts:", file=sys.stderr, flush=True)
    scripts = [x.script_as_integer() for x in descriptions]

    scripts, indices, chunk_size = ucd.deduplicate(scripts)
    scripts_bytes, script_width = ucd.bits_as_bytes(scripts)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #scripts={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(scripts), script_width,
        len(indices_bytes) + len(scripts_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        script_width=script_width,
        scripts_bytes=scripts_bytes
    )

def generate_bidi_classes(template_path, output_path, descriptions):
    print("Processing bidi_classes:", file=sys.stderr, flush=True)
    bidi_classes = [x.bidi_class_as_integer() for x in descriptions]

    bidi_classes, indices, chunk_size = ucd.deduplicate(bidi_classes)
    bidi_classes_bytes, bidi_class_width = ucd.bits_as_bytes(bidi_classes)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #bidi_classes={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(bidi_classes), bidi_class_width,
        len(indices_bytes) + len(bidi_classes_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        bidi_class_width=bidi_class_width,
        bidi_classes_bytes=bidi_classes_bytes
    )

def generate_bidi_paired_bracket_types(template_path, output_path, descriptions):
    print("Processing bidi_paired_bracket_types:", file=sys.stderr, flush=True)
    bidi_paired_bracket_types = [x.bidi_paired_bracket_type_as_integer() for x in descriptions]

    bidi_paired_bracket_types, indices, chunk_size = ucd.deduplicate(bidi_paired_bracket_types)
    bidi_paired_bracket_types_bytes, bidi_paired_bracket_type_width = ucd.bits_as_bytes(bidi_paired_bracket_types)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #bidi_paired_bracket_types={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(bidi_paired_bracket_types), bidi_paired_bracket_type_width,
        len(indices_bytes) + len(bidi_paired_bracket_types_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        bidi_paired_bracket_type_width=bidi_paired_bracket_type_width,
        bidi_paired_bracket_types_bytes=bidi_paired_bracket_types_bytes
    )

def generate_bidi_mirroring_glyphs(template_path, output_path, descriptions):
    print("Processing bidi_mirroring_glyphs:", file=sys.stderr, flush=True)
    bidi_mirroring_glyphs = [x.bidi_mirroring_glyph or 0 for x in descriptions]

    bidi_mirroring_glyphs, indices, chunk_size = ucd.deduplicate(bidi_mirroring_glyphs)
    bidi_mirroring_glyphs_bytes, bidi_mirroring_glyph_width = ucd.bits_as_bytes(bidi_mirroring_glyphs)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #bidi_mirroring_glyphs={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(bidi_mirroring_glyphs), bidi_mirroring_glyph_width,
        len(indices_bytes) + len(bidi_mirroring_glyphs_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        bidi_mirroring_glyph_width=bidi_mirroring_glyph_width,
        bidi_mirroring_glyphs_bytes=bidi_mirroring_glyphs_bytes
    )

def generate_east_asian_widths(template_path, output_path, descriptions):
    print("Processing east_asian_widths:", file=sys.stderr, flush=True)
    east_asian_widths = [x.east_asian_width_as_integer() for x in descriptions]

    east_asian_widths, indices, chunk_size = ucd.deduplicate(east_asian_widths)
    east_asian_widths_bytes, east_asian_width_width = ucd.bits_as_bytes(east_asian_widths)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #east_asian_widths={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(east_asian_widths), east_asian_width_width,
        len(indices_bytes) + len(east_asian_widths_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        east_asian_width_width=east_asian_width_width,
        east_asian_widths_bytes=east_asian_widths_bytes
    )

def generate_canonical_combining_classes(template_path, output_path, descriptions):
    print("Processing canonical_combining_class:", file=sys.stderr, flush=True)
    canonical_combining_classes = [x.canonical_combining_class for x in descriptions]

    canonical_combining_classes, indices, chunk_size = ucd.deduplicate(canonical_combining_classes)
    canonical_combining_classes_bytes, canonical_combining_class_width = ucd.bits_as_bytes(canonical_combining_classes)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #canonical_combining_classes={}:{} total={} bytes".format(
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

def generate_grapheme_cluster_breaks(template_path, output_path, descriptions):
    print("Processing grapheme_cluster_break:", file=sys.stderr, flush=True)
    grapheme_cluster_breaks = [x.grapheme_cluster_break_as_integer() for x in descriptions]

    grapheme_cluster_breaks, indices, chunk_size = ucd.deduplicate(grapheme_cluster_breaks)
    grapheme_cluster_breaks_bytes, grapheme_cluster_break_width = ucd.bits_as_bytes(grapheme_cluster_breaks)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #grapheme_cluster_breaks={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(grapheme_cluster_breaks), grapheme_cluster_break_width,
        len(indices_bytes) + len(grapheme_cluster_breaks_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        grapheme_cluster_break_width=grapheme_cluster_break_width,
        grapheme_cluster_breaks_bytes=grapheme_cluster_breaks_bytes
    )

def generate_line_break_classes(template_path, output_path, descriptions):
    print("Processing line_break_class:", file=sys.stderr, flush=True)
    line_break_classes = [x.line_break_class_as_integer() for x in descriptions]

    line_break_classes, indices, chunk_size = ucd.deduplicate(line_break_classes)
    line_break_classes_bytes, line_break_class_width = ucd.bits_as_bytes(line_break_classes)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #line_break_classes={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(line_break_classes), line_break_class_width,
        len(indices_bytes) + len(line_break_classes_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        line_break_class_width=line_break_class_width,
        line_break_classes_bytes=line_break_classes_bytes
    )

def generate_word_break_properties(template_path, output_path, descriptions):
    print("Processing word_break_property:", file=sys.stderr, flush=True)
    word_break_properties = [x.word_break_property_as_integer() for x in descriptions]

    word_break_properties, indices, chunk_size = ucd.deduplicate(word_break_properties)
    word_break_properties_bytes, word_break_property_width = ucd.bits_as_bytes(word_break_properties)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #word_break_properties={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(word_break_properties), word_break_property_width,
        len(indices_bytes) + len(word_break_properties_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        word_break_property_width=word_break_property_width,
        word_break_properties_bytes=word_break_properties_bytes
    )

def generate_sentence_break_properties(template_path, output_path, descriptions):
    print("Processing sentence_break_property:", file=sys.stderr, flush=True)
    sentence_break_properties = [x.sentence_break_property_as_integer() for x in descriptions]

    sentence_break_properties, indices, chunk_size = ucd.deduplicate(sentence_break_properties)
    sentence_break_properties_bytes, sentence_break_property_width = ucd.bits_as_bytes(sentence_break_properties)
    indices_bytes, index_width = ucd.bits_as_bytes(indices)

    print("    chunk-size={} #indices={}:{} #sentence_break_properties={}:{} total={} bytes".format(
        chunk_size,
        len(indices), index_width,
        len(sentence_break_properties), sentence_break_property_width,
        len(indices_bytes) + len(sentence_break_properties_bytes)),
        file=sys.stderr)

    ucd.generate_output(
        template_path,
        output_path,
        chunk_size=chunk_size,
        indices_size=len(indices),
        index_width=index_width,
        indices_bytes=indices_bytes,
        sentence_break_property_width=sentence_break_property_width,
        sentence_break_properties_bytes=sentence_break_properties_bytes
    )

def generate_decompositions(template_path, output_path, descriptions):
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
    ucd.parse_line_break(options.line_break_class_path, descriptions)
    ucd.parse_scripts(options.scripts_path, descriptions)
    ucd.parse_sentence_break_property(options.sentence_break_property_path, descriptions)
    ucd.parse_unicode_data(options.unicode_data_path, descriptions)
    ucd.parse_word_break_property(options.word_break_property_path, descriptions)
    ucd.add_hangul_decompositions(descriptions)

    generate_general_categories(options.general_categories_template_path, options.general_categories_output_path, descriptions)
    generate_scripts(options.scripts_template_path, options.scripts_output_path, descriptions)
    generate_bidi_classes(options.bidi_classes_template_path, options.bidi_classes_output_path, descriptions)
    generate_bidi_paired_bracket_types(options.bidi_paired_bracket_types_template_path, options.bidi_paired_bracket_types_output_path, descriptions)
    generate_bidi_mirroring_glyphs(options.bidi_mirroring_glyphs_template_path, options.bidi_mirroring_glyphs_output_path, descriptions)
    generate_east_asian_widths(options.east_asian_widths_template_path, options.east_asian_widths_output_path, descriptions)
    generate_grapheme_cluster_breaks(options.grapheme_cluster_breaks_template_path, options.grapheme_cluster_breaks_output_path, descriptions)
    generate_line_break_classes(options.line_break_classes_template_path, options.line_break_classes_output_path, descriptions)
    generate_word_break_properties(options.word_break_properties_template_path, options.word_break_properties_output_path, descriptions)
    generate_sentence_break_properties(options.sentence_break_properties_template_path, options.sentence_break_properties_output_path, descriptions)
    generate_canonical_combining_classes(options.canonical_combining_classes_template_path, options.canonical_combining_classes_output_path, descriptions)
    generate_decompositions(options.decompositions_template_path, options.decompositions_output_path, descriptions)
    generate_compositions(options.compositions_template_path, options.compositions_output_path, descriptions)


if __name__ == "__main__":
    main()
