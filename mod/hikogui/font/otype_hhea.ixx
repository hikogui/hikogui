// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <span>
#include <cstddef>

export module hikogui_font_otype_hhea;
import hikogui_font_otype_utilities;
import hikogui_utility;

export namespace hi { inline namespace v1 {

[[nodiscard]] auto otype_hhea_parse(std::span<std::byte const> bytes, float em_scale)
{
    struct header_type {
        big_int16_buf_t major_version;
        big_int16_buf_t minor_version;
        otype_fword_buf_t ascender;
        otype_fword_buf_t descender;
        otype_fword_buf_t line_gap;
        otype_fuword_buf_t advance_width_max;
        otype_fword_buf_t min_left_side_bearing;
        otype_fword_buf_t min_right_side_bearing;
        otype_fword_buf_t x_max_extent;
        big_int16_buf_t caret_slope_rise;
        big_int16_buf_t caret_slop_run;
        big_int16_buf_t caret_offset;
        big_int16_buf_t reserved_0;
        big_int16_buf_t reserved_1;
        big_int16_buf_t reserved_2;
        big_int16_buf_t reserved_3;
        big_int16_buf_t metric_data_format;
        big_uint16_buf_t number_of_h_metrics;
    };

    struct return_type {
        float ascender;
        float descender;
        float line_gap;
        uint16_t number_of_h_metrics;
    };

    hilet& header = implicit_cast<header_type>(bytes);
    hi_check(*header.major_version == 1 && *header.minor_version == 0, "'hhea' version is not 1.0");

    auto r = return_type{};
    r.ascender = header.ascender * em_scale;
    r.descender = header.descender * em_scale;
    r.line_gap = header.line_gap * em_scale;
    r.number_of_h_metrics = *header.number_of_h_metrics;

    hi_check(r.ascender > 0.0f, "'hhea' ascender must be larger than 0.0");
    hi_check(r.descender <= 0.0f, "'hhea' descender must be less than or equal to 0.0");
    return r;
}

}} // namespace hi::v1
