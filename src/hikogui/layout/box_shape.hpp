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
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    std::optional<int> baseline = {};
    std::optional<int> centerline = {};

    constexpr box_shape() noexcept = default;
    constexpr box_shape(box_shape const&) noexcept = default;
    constexpr box_shape(box_shape&&) noexcept = default;
    constexpr box_shape& operator=(box_shape const&) noexcept = default;
    constexpr box_shape& operator=(box_shape&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(box_shape const&, box_shape const&) noexcept = default;

    constexpr box_shape(extent2 size) noexcept :
        x(0), y(0), width(narrow_cast<int>(size.width())), height(narrow_cast<int>(size.height())), baseline(), centerline()
    {
    }

    constexpr box_shape(box_constraints const& constraints, aarectangle const& rectangle, int baseline_adjustment) noexcept :
        x(narrow_cast<int>(rectangle.x())),
        y(narrow_cast<int>(rectangle.y())),
        width(narrow_cast<int>(rectangle.width())),
        height(narrow_cast<int>(rectangle.height())),
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
        hi_axiom(width >= constraints.minimum_width);
        hi_axiom(height >= constraints.minimum_height);
    }

    [[nodiscard]] constexpr extent2 size() const noexcept
    {
        return {narrow_cast<float>(width), narrow_cast<float>(height)};
    }

    [[nodiscard]] constexpr aarectangle rectangle() const noexcept
    {
        return {narrow_cast<float>(x), narrow_cast<float>(y), narrow_cast<float>(width), narrow_cast<float>(height)};
    }
};

}} // namespace hi::v1