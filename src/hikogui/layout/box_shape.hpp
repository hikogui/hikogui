// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "box_constraints.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include <limits>

namespace hi { inline namespace v1 {

struct box_shape {
    aarectangle rectangle = {};
    float base_line = std::numeric_limits<float>::quiet_NaN();
    float decimal_line = std::numeric_limits<float>::quiet_NaN();

    constexpr box_shape() noexcept = default;
    constexpr box_shape(box_shape const&) noexcept = default;
    constexpr box_shape(box_shape&&) noexcept = default;
    constexpr box_shape& operator=(box_shape const&) noexcept = default;
    constexpr box_shape& operator=(box_shape&&) noexcept = default;

    constexpr box_shape(box_constraints const& constraints, aarectangle const& rectangle, float x_height) noexcept :
        rectangle(rectangle),
        base_line(make_base_line(
            constraints.alignment.vertical(),
            get<0>(rectangle).y(),
            get<3>(rectangle).y(),
            constraints.padding.bottom(),
            constraints.padding.top(),
            x_height)),
        decimal_line(make_decimal_line(
            constraints.alignment.horizontal(),
            get<0>(rectangle).x(),
            get<3>(rectangle).x(),
            constraints.padding.left(),
            constraints.padding.right()))
    {
        hi_axiom(rectangle.size() >= constraints.minimum);
    }

    [[nodiscard]] constexpr static float make_base_line(
        vertical_alignment alignment,
        float y_min,
        float y_max,
        float padding_top,
        float padding_bottom,
        float x_height) noexcept
    {
        hilet bottom_base_line = y_min + padding_bottom;
        hilet top_base_line = y_max - padding_top - x_height;
        hilet middle_base_line = (y_min + y_max) / 2.0f - x_height / 2.0f;
        hi_axiom(bottom_base_line <= top_base_line);

        switch (alignment) {
        case vertical_alignment::none:
            return std::numeric_limits<float>::quiet_NaN();
        case vertical_alignment::top:
            return top_base_line;
        case vertical_alignment::bottom:
            return bottom_base_line;
        case vertical_alignment::middle:
            return std::clamp(middle_base_line, bottom_base_line, top_base_line);
        };
        hi_no_default();
    }

    [[nodiscard]] constexpr static float
    make_decimal_line(horizontal_alignment alignment, float x_min, float x_max, float padding_left, float padding_right) noexcept
    {
        hilet left_decimal_line = x_min + padding_left;
        hilet right_decimal_line = x_max - padding_right;
        hilet center_decimal_line = (x_min + x_max) / 2.0f;
        hi_axiom(left_decimal_line <= right_decimal_line);

        switch (alignment) {
        case horizontal_alignment::none:
            return std::numeric_limits<float>::quiet_NaN();
        case horizontal_alignment::left:
            return left_decimal_line;
        case horizontal_alignment::right:
            return right_decimal_line;
        case horizontal_alignment::center:
        case horizontal_alignment::justified:
            return std::clamp(center_decimal_line, left_decimal_line, right_decimal_line);
        default:
            // At this point `horizontal_alignment::flush` should be resolved to left or right.
            hi_no_default();
        };
    }
};

}} // namespace hi::v1