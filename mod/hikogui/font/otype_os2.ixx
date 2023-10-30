// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;

#include <span>
#include <cstddef>

export module hikogui_font_otype_os2;
import hikogui_font_font_weight;
import hikogui_font_otype_utilities;
import hikogui_utility;

export namespace hi { inline namespace v1 {

[[nodiscard]] auto otype_parse_os2(std::span<std::byte const> bytes, float em_scale)
{
    struct panose_table {
        uint8_t family_type;
        uint8_t serif_style;
        uint8_t weight;
        uint8_t proportion;
        uint8_t contrast;
        uint8_t stroke_variation;
        uint8_t arm_style;
        uint8_t letterform;
        uint8_t midline;
        uint8_t x_height;
    };

    struct header_type_0 {
        big_uint16_buf_t version;
        big_int16_buf_t avg_char_width;
        big_uint16_buf_t weight_class;
        big_uint16_buf_t width_class;
        big_uint16_buf_t type;
        big_int16_buf_t subscript_x_size;
        big_int16_buf_t subscript_y_size;
        big_int16_buf_t subscript_x_offset;
        big_int16_buf_t subscript_y_offset;
        big_int16_buf_t superscript_x_size;
        big_int16_buf_t superscript_y_size;
        big_int16_buf_t superscript_x_offset;
        big_int16_buf_t superscript_y_offset;
        big_int16_buf_t strikeout_size;
        big_int16_buf_t strikeout_position;
        big_int16_buf_t family_class;
        panose_table panose;
        big_uint32_buf_t unicode_range_1;
        big_uint32_buf_t unicode_range_2;
        big_uint32_buf_t unicode_range_3;
        big_uint32_buf_t unicode_range_4;
        big_uint32_buf_t ach_vend_id;
        big_uint16_buf_t selection;
        big_uint16_buf_t first_char_index;
        big_uint16_buf_t last_char_index;
    };

    struct header_type_2 : header_type_0 {
        big_int16_buf_t typo_ascender;
        big_int16_buf_t typo_descender;
        big_int16_buf_t typo_line_gap;
        big_uint16_buf_t win_ascent;
        big_uint16_buf_t win_descent;
        big_uint32_buf_t code_page_range_1;
        big_uint32_buf_t code_page_range_2;
        otype_fword_buf_t x_height;
        otype_fword_buf_t cap_height;
        big_uint16_buf_t default_char;
        big_uint16_buf_t break_char;
        big_uint16_buf_t max_context;
    };

    struct return_type {
        font_weight weight = font_weight::medium;
        bool condensed = false;
        bool serif = false;
        bool monospace = false;
        bool italic = false;
        float x_height = 0.0f;
        float cap_height = 0.0f;
    };

    hilet& header = implicit_cast<header_type_0>(bytes);
    hi_check(*header.version <= 5, "'OS/2' version must be between 0 and 5");

    auto r = return_type{};

    hilet weight_value = *header.weight_class;
    if (weight_value >= 1 && weight_value <= 1000) {
        r.weight = font_weight_from_int(weight_value);
    }

    hilet width_value = *header.width_class;
    if (width_value >= 1 && width_value <= 4) {
        r.condensed = true;
    } else if (width_value >= 5 && width_value <= 9) {
        r.condensed = false;
    }

    hilet serif_value = header.panose.serif_style;
    if ((serif_value >= 2 && serif_value <= 10) || (serif_value >= 14 && serif_value <= 15)) {
        r.serif = true;
    } else if (serif_value >= 11 && serif_value <= 13) {
        r.serif = false;
    }

    // The Panose weight table is odd, assuming the integer values are
    // increasing with boldness, Thin is bolder then Light.
    // The table below uses the integer value as an indication of boldness.
    switch (header.panose.weight) {
    case 2:
        r.weight = font_weight::thin;
        break;
    case 3:
        r.weight = font_weight::extra_light;
        break;
    case 4:
        r.weight = font_weight::light;
        break;
    case 5:
        r.weight = font_weight::regular;
        break;
    case 6:
        r.weight = font_weight::medium;
        break;
    case 7:
        r.weight = font_weight::semi_bold;
        break;
    case 8:
        r.weight = font_weight::bold;
        break;
    case 9:
        r.weight = font_weight::extra_bold;
        break;
    case 10:
        r.weight = font_weight::black;
        break;
    case 11:
        r.weight = font_weight::extra_black;
        break;
    default:
        break;
    }

    switch (header.panose.proportion) {
    case 2:
    case 3:
    case 4:
    case 5:
    case 7:
        r.monospace = false;
        r.condensed = false;
        break;
    case 6:
    case 8:
        r.monospace = false;
        r.condensed = true;
        break;
    case 9:
        r.monospace = true;
        r.condensed = false;
        break;
    }

    hilet letterform_value = header.panose.letterform;
    if (letterform_value >= 2 && letterform_value <= 8) {
        r.italic = false;
    } else if (letterform_value >= 9 && letterform_value <= 15) {
        r.italic = true;
    }

    if (*header.version >= 2) {
        hilet &header_v2 = implicit_cast<header_type_2>(bytes);

        r.x_height = header_v2.x_height * em_scale;
        r.cap_height = header_v2.cap_height * em_scale;
    }

    return r;
}

}} // namespace hi::v1
