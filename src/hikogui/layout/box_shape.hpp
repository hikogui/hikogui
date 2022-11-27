// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "box_constraints.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include <limits>
#include <optional>

namespace hi { inline namespace v1 {

struct box_shape {
    int left = 0;
    int right = 0;
    int bottom = 0;
    int top = 0;
    std::optional<int> baseline = {};
    std::optional<int> decimal_line = {};

    constexpr box_shape() noexcept = default;
    constexpr box_shape(box_shape const&) noexcept = default;
    constexpr box_shape(box_shape&&) noexcept = default;
    constexpr box_shape& operator=(box_shape const&) noexcept = default;
    constexpr box_shape& operator=(box_shape&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(box_shape const &, box_shape const &) noexcept = default;

    constexpr box_shape(extent2 size) noexcept :
        left(0),
        bottom(0),
        right(narrow_cast<int>(size.width())),
        top(narrow_cast<int>(size.height())),
        baseline(),
        decimal_line()
    {
    }

    constexpr box_shape(box_constraints const& constraints, aarectangle const& rectangle, int baseline_adjustment) noexcept :
        left(narrow_cast<int>(rectangle.left())),
        right(narrow_cast<int>(rectangle.right())),
        baseline(make_baseline(
            constraints.alignment.vertical(),
            narrow_cast<int>(rectangle.bottom()),
            narrow_cast<int>(rectangle.top()),
            constraints.padding_bottom,
            constraints.padding_top,
            baseline_adjustment)),
        decimal_line(make_decimal_line(
            constraints.alignment.horizontal(),
            narrow_cast<int>(rectangle.left()),
            narrow_cast<int>(rectangle.right()),
            constraints.padding_left,
            constraints.padding_right))
    {
        hi_axiom(right - left >= constraints.minimum_width);
        hi_axiom(top - bottom >= constraints.minimum_height);
    }

    [[nodiscard]] constexpr int width() const noexcept
    {
        hi_axiom(right >= left);
        return right - left;
    }

    [[nodiscard]] constexpr int height() const noexcept
    {
        hi_axiom(top >= bottom);
        return top - bottom;
    }

    [[nodiscard]] constexpr extent2 size() const noexcept
    {
        return {narrow_cast<float>(width()), narrow_cast<float>(height())};
    }

    [[nodiscard]] constexpr aarectangle rectangle() const noexcept
    {
        return {
            point2{narrow_cast<float>(left), narrow_cast<float>(bottom)},
            point2{narrow_cast<float>(right), narrow_cast<float>(top)}};
    }

private:
    [[nodiscard]] constexpr static std::optional<int> make_baseline(
        vertical_alignment alignment,
        int bottom,
        int top,
        int padding_top,
        int padding_bottom,
        int alignment_offset) noexcept
    {
        hi_axiom(top >= bottom);
        hi_axiom(padding_top >= 0);
        hi_axiom(padding_bottom >= 0);
        hi_axiom(alignment_offset >= 0);

        hilet bottom_baseline = bottom + padding_bottom;
        hilet top_baseline = top - padding_top - alignment_offset;
        hilet middle_baseline = (bottom + top) / 2 - alignment_offset / 2;
        hi_axiom(bottom_baseline <= top_baseline);

        switch (alignment) {
        case vertical_alignment::none:
            return {};
        case vertical_alignment::top:
            return top_baseline;
        case vertical_alignment::bottom:
            return bottom_baseline;
        case vertical_alignment::middle:
            return std::clamp(middle_baseline, bottom_baseline, top_baseline);
        };
        hi_no_default();
    }

    [[nodiscard]] constexpr static std::optional<int>
    make_decimal_line(horizontal_alignment alignment, int left, int right, int padding_left, int padding_right) noexcept
    {
        hi_axiom(right >= left);
        hi_axiom(padding_left >= 0);
        hi_axiom(padding_right >= 0);

        hilet left_decimal_line = left + padding_left;
        hilet right_decimal_line = right - padding_right;
        hilet center_decimal_line = (left + right) / 2;
        hi_axiom(left_decimal_line <= right_decimal_line);

        switch (alignment) {
        case horizontal_alignment::none:
            return {};
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