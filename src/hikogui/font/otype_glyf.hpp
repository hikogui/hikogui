// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "otype_utilities.hpp"
#include "font_char_map.hpp"
#include "../coroutine/coroutine.hpp"
#include "../macros.hpp"
#include <variant>
#include <coroutine>

hi_export_module(hikogui.font.otype_glyf);

hi_export namespace hi { inline namespace v1 {
namespace detail {
constexpr uint16_t otype_glyf_flag_arg1_and_arg2_are_words = 0x0001;
constexpr uint16_t otype_glyf_flag_args_are_xy_values = 0x0002;
constexpr uint16_t otype_glyf_flag_round_xy_to_grid = 0x0004;
constexpr uint16_t otype_glyf_flag_has_scale = 0x0008;
constexpr uint16_t otype_glyf_flag_more_components = 0x0020;
constexpr uint16_t otype_glyf_flag_has_xy_scale = 0x0040;
constexpr uint16_t otype_glyf_flag_has_2x2 = 0x0080;
constexpr uint16_t otype_glyf_flag_instructions = 0x0100;
constexpr uint16_t otype_glyf_flag_use_this_glyph_metrics = 0x0200;
constexpr uint16_t otype_glyf_flag_overlap_compound = 0x0400;
constexpr uint16_t otype_glyf_flag_scaled_component_offset = 0x0800;
constexpr uint16_t otype_glyf_flag_unscaled_component_offset = 0x1000;

constexpr uint8_t otype_glyf_flag_on_curve = 0x01;
constexpr uint8_t otype_glyf_flag_x_short = 0x02;
constexpr uint8_t otype_glyf_flag_y_short = 0x04;
constexpr uint8_t otype_glyf_flag_repeat = 0x08;
constexpr uint8_t otype_glyf_flag_x_same = 0x10;
constexpr uint8_t otype_glyf_flag_y_same = 0x20;

struct otype_glyf_header {
    big_int16_buf_t num_contours;
    otype_fword_buf_t x_min;
    otype_fword_buf_t y_min;
    otype_fword_buf_t x_max;
    otype_fword_buf_t y_max;
};

} // namespace detail

/** Get the graphic-path of a simple glyph.
 *
 *
 * @note Only call this function when `otype_glyf_is_compound() == false`.
 */
[[nodiscard]] hi_inline graphic_path
otype_glyf_get_path(std::span<std::byte const> bytes, float em_scale)
{
    auto r = graphic_path{};

    if (bytes.empty()) {
        // Empty glyphs have no path, and therefor an empty bounding rectangle.
        return r;
    }

    auto offset = 0_uz;
    hilet& header = implicit_cast<detail::otype_glyf_header>(offset, bytes);
    hilet num_contours = *header.num_contours;

    hi_check(num_contours >= 0, "'glyph' path requested on a compound glyph.");

    // Check includes instructionLength.
    hilet end_points = implicit_cast<big_uint16_buf_t>(offset, bytes, num_contours);

    r.contourEndPoints.reserve(end_points.size());
    uint16_t max_end_point = 0;
    for (hilet end_point : end_points) {
        hilet end_point_ = *end_point;

        hi_check(end_point_ >= max_end_point, "'glyf' end-point indices must be increasing");
        max_end_point = end_point_;

        r.contourEndPoints.push_back(end_point_);
    }

    hilet num_points = wide_cast<size_t>(max_end_point) + 1;

    // Skip over the instructions.
    hilet instruction_size = *implicit_cast<big_uint16_buf_t>(offset, bytes);
    offset += instruction_size;

    // Extract all the flags.
    auto flags = std::vector<uint8_t>(num_points, uint8_t{0});
    for (auto i = 0_uz; i != num_points; ++i) {
        hilet flag = implicit_cast<uint8_t>(offset, bytes);

        flags[i] = flag;
        if (to_bool(flag & detail::otype_glyf_flag_repeat)) {
            hilet repeat = implicit_cast<uint8_t>(offset, bytes);

            hi_check(i + repeat <= num_points, "'glyf' repeating flags out-of-bound");
            for (std::size_t j = 0; j != repeat; ++j) {
                flags[++i] = flag;
            }
        }
    }

    // Get xCoordinates
    auto x_deltas = std::vector<int16_t>(num_points, int16_t{0});
    for (auto i = 0_uz; i != num_points; ++i) {
        hilet flag = flags[i];

        if (to_bool(flag & detail::otype_glyf_flag_x_short)) {
            if (to_bool(flag & detail::otype_glyf_flag_x_same)) {
                x_deltas[i] = wide_cast<int16_t>(implicit_cast<uint8_t>(offset, bytes));
            } else {
                // Negative short.
                x_deltas[i] = -wide_cast<int16_t>(implicit_cast<uint8_t>(offset, bytes));
            }
        } else {
            if (to_bool(flag & detail::otype_glyf_flag_x_same)) {
                x_deltas[i] = 0;
            } else {
                // Long
                x_deltas[i] = *implicit_cast<big_int16_buf_t>(offset, bytes);
            }
        }
    }

    // Get yCoordinates
    auto y_deltas = std::vector<int16_t>(num_points, int16_t{0});
    for (auto i = 0_uz; i != num_points; ++i) {
        hilet flag = flags[i];

        if (to_bool(flag & detail::otype_glyf_flag_y_short)) {
            if (to_bool(flag & detail::otype_glyf_flag_y_same)) {
                y_deltas[i] = wide_cast<int16_t>(implicit_cast<uint8_t>(offset, bytes));
            } else {
                // Negative short.
                y_deltas[i] = -wide_cast<int16_t>(implicit_cast<uint8_t>(offset, bytes));
            }
        } else {
            if (to_bool(flag & detail::otype_glyf_flag_y_same)) {
                y_deltas[i] = 0;
            } else {
                // Long
                y_deltas[i] = *implicit_cast<big_int16_buf_t>(offset, bytes);
            }
        }
    }

    // Create absolute points
    int x = 0;
    int y = 0;
    r.points.reserve(num_points);
    for (auto i = 0_uz; i != num_points; ++i) {
        hilet flag = flags[i];
        x += x_deltas[i];
        y += y_deltas[i];

        hilet type =
            to_bool(flag & detail::otype_glyf_flag_on_curve) ? bezier_point::Type::Anchor : bezier_point::Type::QuadraticControl;

        r.points.emplace_back(x * em_scale, y * em_scale, type);
    }

    return r;
}

struct otype_glyf_component {
    hi::glyph_id glyph_id = {};
    vector2 offset = {};
    matrix2 scale = {};

    /** The point-nr in the compound being assembled.
     */
    size_t compound_point_index = 0;

    /** The point in the current component being added to the compound.
     */
    size_t component_point_index = 0;

    /** The component is positioned using anchor points.
     */
    bool use_points = false;

    /** Use this glyph for the metrics of the compound.
     */
    bool use_for_metrics = false;
};

/** Get the components of a compound glyph.
 *
 *
 * @note Only call this function when `otype_glyf_is_compound() == true`.
 */
[[nodiscard]] hi_inline generator<otype_glyf_component> otype_glyf_get_compound(std::span<std::byte const> bytes, float em_scale)
{
    if (bytes.empty()) {
        // Empty glyphs have no path, and therefor an empty bounding rectangle.
        co_return;
    }

    auto offset = 0_uz;
    hilet& header = implicit_cast<detail::otype_glyf_header>(offset, bytes);

    hi_check(*header.num_contours < 0, "'glyph' compound requested on a simple glyph.");

    uint16_t flags;
    do {
        flags = *implicit_cast<big_uint16_buf_t>(offset, bytes);

        auto component = otype_glyf_component{};
        component.glyph_id = glyph_id{*implicit_cast<big_uint16_buf_t>(offset, bytes)};

        if (to_bool(flags & detail::otype_glyf_flag_args_are_xy_values)) {
            if (to_bool(flags & detail::otype_glyf_flag_arg1_and_arg2_are_words)) {
                hilet tmp = implicit_cast<otype_fword_buf_t>(offset, bytes, 2);
                component.offset = vector2{tmp[0] * em_scale, tmp[1] * em_scale};
            } else {
                hilet tmp = implicit_cast<otype_fbyte_buf_t>(offset, bytes, 2);
                component.offset = vector2{tmp[0] * em_scale, tmp[1] * em_scale};
            }
        } else {
            component.use_points = true;
            if (to_bool(flags & detail::otype_glyf_flag_arg1_and_arg2_are_words)) {
                hilet tmp = implicit_cast<big_uint16_buf_t>(offset, bytes, 2);
                component.compound_point_index = *tmp[0];
                component.component_point_index = *tmp[1];
            } else {
                hilet tmp = implicit_cast<uint8_t>(offset, bytes, 2);
                component.compound_point_index = tmp[0];
                component.component_point_index = tmp[1];
            }
        }

        // Start with an identity matrix.
        if (to_bool(flags & detail::otype_glyf_flag_has_scale)) {
            component.scale = scale2(*implicit_cast<otype_fixed1_14_buf_t>(offset, bytes));

        } else if (to_bool(flags & detail::otype_glyf_flag_has_xy_scale)) {
            hilet tmp = implicit_cast<otype_fixed1_14_buf_t>(offset, bytes, 2);
            component.scale = scale2(*tmp[0], *tmp[1]);

        } else if (to_bool(flags & detail::otype_glyf_flag_has_2x2)) {
            hilet tmp = implicit_cast<otype_fixed1_14_buf_t>(offset, bytes, 4);
            component.scale = matrix2{vector2{*tmp[0], *tmp[1]}, vector2{*tmp[2], *tmp[3]}};
        }

        if (to_bool(flags & detail::otype_glyf_flag_scaled_component_offset)) {
            component.offset = component.scale * component.offset;
        }

        if (to_bool(flags & detail::otype_glyf_flag_use_this_glyph_metrics)) {
            component.use_for_metrics = true;
        }

        co_yield component;

    } while (to_bool(flags & detail::otype_glyf_flag_more_components));
    // Ignore trailing instructions.
}

/** Check if this glyph is a compound or simple glyph.
 *
 */
[[nodiscard]] hi_inline bool otype_glyf_is_compound(std::span<std::byte const> bytes)
{
    struct header_type {
        big_int16_buf_t num_contours;
        otype_fword_buf_t x_min;
        otype_fword_buf_t y_min;
        otype_fword_buf_t x_max;
        otype_fword_buf_t y_max;
    };

    if (bytes.empty()) {
        // Empty glyphs are simple, non-compound glyphs.
        return false;
    }

    hilet& header = implicit_cast<detail::otype_glyf_header>(bytes);
    return *header.num_contours < 0;
}

/** Get the bounding box of a simple glyph.
 *
 *
 * @note Only call this function when `otype_glyf_is_compound() == false`.
 */
[[nodiscard]] hi_inline aarectangle otype_glyf_get_bounding_box(std::span<std::byte const> bytes, float em_scale)
{
    struct header_type {
        big_int16_buf_t num_contours;
        otype_fword_buf_t x_min;
        otype_fword_buf_t y_min;
        otype_fword_buf_t x_max;
        otype_fword_buf_t y_max;
    };

    if (bytes.empty()) {
        // Empty glyphs have no path, and therefor an empty bounding rectangle.
        return aarectangle{};
    }

    hilet& header = implicit_cast<detail::otype_glyf_header>(bytes);

    hilet x_min = header.x_min * em_scale;
    hilet y_min = header.y_min * em_scale;
    hilet x_max = header.x_max * em_scale;
    hilet y_max = header.y_max * em_scale;

    hi_check(x_min <= x_max, "'glyf' bounding box is invalid.");
    hi_check(y_min <= y_max, "'glyf' bounding box is invalid.");

    return aarectangle{point2{x_min, y_min}, point2{x_max, y_max}};
}

}} // namespace hi::v1
