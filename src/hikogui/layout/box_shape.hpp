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
    std::optional<int> centerline = {};

    constexpr box_shape() noexcept = default;
    constexpr box_shape(box_shape const&) noexcept = default;
    constexpr box_shape(box_shape&&) noexcept = default;
    constexpr box_shape& operator=(box_shape const&) noexcept = default;
    constexpr box_shape& operator=(box_shape&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(box_shape const&, box_shape const&) noexcept = default;

    constexpr box_shape(extent2 size) noexcept :
        left(0), bottom(0), right(narrow_cast<int>(size.width())), top(narrow_cast<int>(size.height())), baseline(), centerline()
    {
    }

    constexpr box_shape(box_constraints const& constraints, aarectangle const& rectangle, int baseline_adjustment) noexcept :
        left(narrow_cast<int>(rectangle.left())),
        right(narrow_cast<int>(rectangle.right())),
        bottom(narrow_cast<int>(rectangle.bottom())),
        top(narrow_cast<int>(rectangle.top())),
        baseline(make_guideline(
            constraints.alignment.vertical(),
            narrow_cast<int>(rectangle.bottom()),
            narrow_cast<int>(rectangle.top()),
            constraints.padding_bottom,
            constraints.padding_top,
            baseline_adjustment)),
        centerline(make_guideline(
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
};

}} // namespace hi::v1