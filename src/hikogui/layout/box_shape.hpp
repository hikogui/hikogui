// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "box_constraints.hpp"
#include "../geometry/module.hpp"
#include "../utility/module.hpp"
#include <limits>
#include <optional>

namespace hi { inline namespace v1 {

struct box_shape {
    aarectanglei rectangle;
    std::optional<int> baseline = {};
    std::optional<int> centerline = {};

    constexpr box_shape() noexcept = default;
    constexpr box_shape(box_shape const&) noexcept = default;
    constexpr box_shape(box_shape&&) noexcept = default;
    constexpr box_shape& operator=(box_shape const&) noexcept = default;
    constexpr box_shape& operator=(box_shape&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(box_shape const&, box_shape const&) noexcept = default;

    constexpr box_shape(extent2i size) noexcept : rectangle(size), baseline(), centerline() {}

    constexpr box_shape(
        override_t,
        box_constraints const& constraints,
        aarectanglei const& rectangle,
        int baseline_adjustment) noexcept :
        rectangle(rectangle),
        baseline(make_guideline(
            constraints.alignment.vertical(),
            rectangle.bottom(),
            rectangle.top(),
            constraints.padding.bottom(),
            constraints.padding.top(),
            baseline_adjustment)),
        centerline(make_guideline(
            constraints.alignment.horizontal(),
            rectangle.left(),
            rectangle.right(),
            constraints.padding.left(),
            constraints.padding.right()))
    {
    }

    constexpr box_shape(box_constraints const& constraints, aarectanglei rectangle, int baseline_adjustment) noexcept :
        box_shape(override_t{}, constraints, rectangle, baseline_adjustment)
    {
        hi_axiom(rectangle.size() >= constraints.minimum);
    }

    [[deprecated]] constexpr box_shape(
        override_t,
        box_constraints const& constraints,
        aarectangle const& rectangle,
        int baseline_adjustment) noexcept :
        box_shape(override_t{}, constraints, narrow_cast<aarectanglei>(rectangle), baseline_adjustment)
    {
    }

    [[deprecated]] constexpr box_shape(
        box_constraints const& constraints,
        aarectangle const& rectangle,
        int baseline_adjustment) noexcept :
        box_shape(constraints, narrow_cast<aarectanglei>(rectangle), baseline_adjustment)
    {
    }

    [[nodiscard]] constexpr int x() const noexcept
    {
        return rectangle.x();
    }

    [[nodiscard]] constexpr int y() const noexcept
    {
        return rectangle.y();
    }

    [[nodiscard]] constexpr extent2i size() const noexcept
    {
        return rectangle.size();
    }

    [[nodiscard]] constexpr int width() const noexcept
    {
        return rectangle.width();
    }

    [[nodiscard]] constexpr int height() const noexcept
    {
        return rectangle.height();
    }
};

}} // namespace hi::v1