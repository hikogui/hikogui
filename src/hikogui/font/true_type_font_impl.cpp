// Copyright Take Vos 2019-2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "true_type_font.hpp"
#include "otype_utilities.hpp"
#include "otype_cmap.hpp"
#include "otype_glyf.hpp"
#include "otype_head.hpp"
#include "otype_hhea.hpp"
#include "otype_hmtx.hpp"
#include "otype_kern.hpp"
#include "otype_loca.hpp"
#include "otype_maxp.hpp"
#include "otype_name.hpp"
#include "otype_os2.hpp"
#include "otype_sfnt.hpp"
#include "../geometry/module.hpp"
#include "../utility/module.hpp"
#include "../placement.hpp"
#include "../strings.hpp"
#include "../log.hpp"
#include <cstddef>
#include <span>

namespace hi::inline v1 {

graphic_path true_type_font::get_path(glyph_id glyph_id) const
{
    load_view();

    hi_check(*glyph_id < num_glyphs, "glyph_id is not valid in this font.");

    hilet glyph_bytes = otype_loca_get(_loca_table_bytes, _glyf_table_bytes, glyph_id, _loca_is_offset32);

    if (otype_glyf_is_compound(glyph_bytes)) {
        auto r = graphic_path{};

        for (hilet& component : otype_glyf_get_compound(glyph_bytes, _em_scale)) {
            auto component_path = component.scale * get_path(component.glyph_id);

            if (component.use_points) {
                hilet compound_point = hi_check_at(r.points, component.compound_point_index).p;
                hilet component_point = hi_check_at(component_path.points, component.component_point_index).p;
                hilet offset = translate2{compound_point - component_point};
                component_path = offset * component_path;
            } else {
                component_path = translate2{component.offset} * component_path;
            }

            r += component_path;
        }
        return r;

    } else {
        return otype_glyf_get_path(glyph_bytes, _em_scale);
    }
}

[[nodiscard]] float true_type_font::get_advance(hi::glyph_id glyph_id) const
{
    load_view();

    hi_check(*glyph_id < num_glyphs, "glyph_id is not valid in this font.");
    hilet[advance_width, left_side_bearing] = otype_hmtx_get(_hmtx_table_bytes, glyph_id, _num_horizontal_metrics, _em_scale);
    return advance_width;
}

glyph_metrics true_type_font::get_metrics(hi::glyph_id glyph_id) const
{
    load_view();

    hi_check(*glyph_id < num_glyphs, "glyph_id is not valid in this font.");

    hilet glyph_bytes = otype_loca_get(_loca_table_bytes, _glyf_table_bytes, glyph_id, _loca_is_offset32);

    if (otype_glyf_is_compound(glyph_bytes)) {
        for (hilet& component : otype_glyf_get_compound(glyph_bytes, _em_scale)) {
            if (component.use_for_metrics) {
                return get_metrics(component.glyph_id);
            }
        }
    }

    auto r = glyph_metrics{};
    r.bounding_rectangle = otype_glyf_get_bounding_box(glyph_bytes, _em_scale);
    hilet[advance_width, left_side_bearing] = otype_hmtx_get(_hmtx_table_bytes, glyph_id, _num_horizontal_metrics, _em_scale);

    r.advance = advance_width;
    r.left_side_bearing = left_side_bearing;
    r.right_side_bearing = advance_width - (left_side_bearing + r.bounding_rectangle.width());
    return r;
}

void true_type_font::parse_font_directory(std::span<std::byte const> bytes)
{
    if (auto head_bytes = otype_sfnt_search<"head">(bytes); not head_bytes.empty()) {
        auto head = otype_head_parse(head_bytes);
        _loca_is_offset32 = head.loca_is_offset32;
        _em_scale = head.em_scale;
    }

    if (auto name_bytes = otype_sfnt_search<"name">(bytes); not name_bytes.empty()) {
        auto names = otype_name_get_family(name_bytes);
        family_name = std::move(names.family_name);
        sub_family_name = std::move(names.sub_family_name);
    }

    if (auto maxp_bytes = otype_sfnt_search<"maxp">(bytes); not maxp_bytes.empty()) {
        auto maxp = otype_maxp_parse(maxp_bytes);
        num_glyphs = maxp.num_glyphs;
    }

    if (auto hhea_bytes = otype_sfnt_search<"hhea">(bytes); not hhea_bytes.empty()) {
        auto hhea = otype_hhea_parse(hhea_bytes, _em_scale);
        metrics.ascender = hhea.ascender;
        metrics.descender = -hhea.descender;
        metrics.line_gap = hhea.line_gap;
        _num_horizontal_metrics = hhea.number_of_h_metrics;
    }

    if (auto cmap_bytes = otype_sfnt_search<"cmap">(bytes); not cmap_bytes.empty()) {
        char_map = otype_cmap_parse(cmap_bytes);
    } else {
        throw parse_error("Could not find 'cmap'");
    }

    if (auto os2_bytes = otype_sfnt_search<"OS/2">(bytes); not os2_bytes.empty()) {
        auto os2 = otype_parse_os2(os2_bytes, _em_scale);
        weight = os2.weight;
        condensed = os2.condensed;
        serif = os2.serif;
        monospace = os2.monospace;
        italic = os2.italic;
        OS2_x_height = os2.x_height;
        OS2_cap_height = os2.cap_height;
    }

    cache_tables(bytes);

    // Parsing the weight, italic and other features from the sub-family-name
    // is much more reliable than the explicit data in the OS/2 table.
    // Only use the OS/2 data as a last resort.
    // clang-format off
    auto name_lower = to_lower(family_name + " " + sub_family_name);
    if (name_lower.find("italic") != std::string::npos or
        name_lower.find("oblique") != std::string::npos) {
        italic = true;
    }

    if (name_lower.find("condensed") != std::string::npos) {
        condensed = true;
    }

    if (name_lower.find("mono") != std::string::npos or
        name_lower.find("console") != std::string::npos or
        name_lower.find("code") != std::string::npos ) {
        monospace = true;
    }

    if (name_lower.find("sans") != std::string::npos) {
        serif = false;
    } else if (name_lower.find("serif") != std::string::npos) {
        serif = true;
    }

    if (name_lower.find("regular") != std::string::npos or
        name_lower.find("medium") != std::string::npos) {
        weight = font_weight::Regular;
    } else if (
        name_lower.find("extra light") != std::string::npos or
        name_lower.find("extra-light") != std::string::npos or
        name_lower.find("extralight") != std::string::npos) {
        weight = font_weight::ExtraLight;
    } else if (
        name_lower.find("extra black") != std::string::npos or
        name_lower.find("extra-black") != std::string::npos or
        name_lower.find("extrablack") != std::string::npos) {
        weight = font_weight::ExtraBlack;
    } else if (
        name_lower.find("extra bold") != std::string::npos or
        name_lower.find("extra-bold") != std::string::npos or
        name_lower.find("extrabold") != std::string::npos) {
        weight = font_weight::ExtraBold;
    } else if (name_lower.find("thin") != std::string::npos) {
        weight = font_weight::Thin;
    } else if (name_lower.find("light") != std::string::npos) {
        weight = font_weight::Light;
    } else if (name_lower.find("bold") != std::string::npos) {
        weight = font_weight::Bold;
    } else if (name_lower.find("black") != std::string::npos) {
        weight = font_weight::Black;
    }
    // clang-format on

    // Figure out the features.
    features.clear();
    if (not _kern_table_bytes.empty()) {
        features += "kern,";
    }
    if (not _GSUB_table_bytes.empty()) {
        features += "GSUB,";
    }

    if (OS2_x_height > 0.0f) {
        metrics.x_height = OS2_x_height;
    } else {
        hilet glyph_id = find_glyph('x');
        if (glyph_id) {
            metrics.x_height = get_metrics(glyph_id).bounding_rectangle.height();
        }
    }

    if (OS2_cap_height > 0.0f) {
        metrics.cap_height = OS2_cap_height;
    } else {
        hilet glyph_id = find_glyph('H');
        if (glyph_id) {
            metrics.cap_height = get_metrics(glyph_id).bounding_rectangle.height();
        }
    }

    hilet glyph_id = find_glyph('8');
    if (glyph_id) {
        metrics.digit_advance = get_metrics(glyph_id).advance;
    }
}

[[nodiscard]] font::shape_run_result_type true_type_font::shape_run_basic(gstring run) const
{
    auto r = font::shape_run_result_type{};
    r.reserve(run.size());

    auto x = 0.0f;
    for (hilet grapheme : run) {
        hilet glyphs = find_glyph(grapheme);
        auto grapheme_advance = 0.0f;
        for (hilet glyph_id : glyphs) {
            hilet glyph_metrics = get_metrics(glyph_id);
            hilet glyph_bounding_rectangle = translate2{x, 0.0f} * glyph_metrics.bounding_rectangle;
            hilet glyph_position = point2{x, 0.0f};

            x += glyph_metrics.advance;
            grapheme_advance += glyph_metrics.advance;

            r.glyphs.push_back(glyph_id);
            r.glyph_positions.push_back(glyph_position);
            r.glyph_bounding_rectangles.push_back(glyph_bounding_rectangle);
        }
        r.grapheme_advances.push_back(grapheme_advance);
        r.glyph_count.push_back(glyphs.size());
    }
    return r;
}

void true_type_font::shape_run_kern(font::shape_run_result_type& shape_result) const
{
    hilet num_graphemes = shape_result.grapheme_advances.size();

    auto total_kerning = translate2{};
    auto prev_glyph_id = hi::glyph_id{};
    auto glyph_index = 0_uz;
    for (auto i = 0_uz; i != num_graphemes; ++i) {
        hilet num_glyphs_in_grapheme = shape_result.glyph_count[i];
        for (auto j = 0_uz; j != num_glyphs_in_grapheme; ++j, ++glyph_index) {
            auto glyph_id = shape_result.glyphs[glyph_index];

            if (prev_glyph_id) {
                auto kerning = otype_kern_find(_kern_table_bytes, prev_glyph_id, glyph_id, _em_scale);
                if (kerning.y() != 0.0f) {
                    throw parse_error("'kern' table contains vertical kerning.");
                }
                total_kerning.x() += kerning.x();
            }

            shape_result.glyph_bounding_rectangles[glyph_index] *= total_kerning;
            shape_result.glyph_positions[glyph_index] *= total_kerning;

            prev_glyph_id = glyph_id;
        }

        shape_result.grapheme_advances[i] += total_kerning.x();
    }
}

[[nodiscard]] font::shape_run_result_type true_type_font::shape_run(iso_639 language, iso_15924 script, gstring run) const
{
    auto r = shape_run_basic(run);

    // Glyphs should be morphed only once.
    auto morphed = false;
    // Glyphs should be positioned only once.
    auto positioned = false;

    if (not positioned and not _kern_table_bytes.empty()) {
        try {
            shape_run_kern(r);
            positioned = true;
        } catch (std::exception const& e) {
            hi_log_error("Turning off invalid 'kern' table in font '{} {}': {}", family_name, sub_family_name, e.what());
            _kern_table_bytes = {};
        }
    }

    return r;
}

} // namespace hi::inline v1
