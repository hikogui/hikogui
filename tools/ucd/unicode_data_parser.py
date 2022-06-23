

from .ucd_parser import parse_ucd

def parse_unicode_data(filename, descriptions):
    first_code_point = None
    for columns in parse_ucd(filename):
        code_points = columns[0]
        name = columns[1]
        general_category = columns[2]
        canonical_combining_class = int(columns[3], 10)
        bidi_class = columns[4]
        decomposition_type_mapping = columns[5]
        bidi_mirrored = columns[9] == "Y"
        simple_upper_case_mapping = int(columns[12], 16) if columns[12] else None
        simple_lower_case_mapping = int(columns[13], 16) if columns[13] else None
        simple_title_case_mapping = int(columns[14], 16) if columns[14] else None

        if decomposition_type_mapping == "":
            decomposition_type = "none"
            decomposition_mapping = []
        elif decomposition_type_mapping[0] == "<":
            [t, m] = decomposition_type_mapping.split(">")
            decomposition_type = t[1:]
            decomposition_mapping = [int(x, 16) for x in m.strip().split(" ")]
        else:
            decomposition_type = "canonical"
            decomposition_mapping = [int(x, 16) for x in decomposition_type_mapping.split(" ")]

        if name.endswith("First>"):
            # We start populating the descriptions when the name ends with "Last>".
            # Therefor we skip "First>" and only take the first code point.
            first_code_point = code_points[0]
            continue
        elif name.endswith("Last>"):
            code_points = range(first_code_point, code_points[0] + 1)
        else:
            first_code_point = None

        for code_point in code_points:
            d = descriptions[code_point]
            d.general_category = general_category
            d.canonical_combining_class = canonical_combining_class;
            d.bidi_class = bidi_class
            d.decomposition_type = decomposition_type
            d.decomposition_mapping = decomposition_mapping
            d.bidi_mirrored = bidi_mirrored
            if simple_upper_case_mapping:
                d.upper_cast_mapping = [simple_upper_case_mapping]
            if simple_lower_case_mapping:
                d.lower_cast_mapping = [simple_lower_case_mapping]
            if simple_title_case_mapping:
                d.title_cast_mapping = [simple_title_case_mapping]

