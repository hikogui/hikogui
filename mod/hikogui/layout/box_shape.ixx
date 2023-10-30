// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <limits>
#include <optional>

export module hikogui_layout_box_shape;
import hikogui_geometry;
import hikogui_layout_box_constraints;
import hikogui_utility;

export namespace hi { inline namespace v1 {

struct box_shape {
    aarectangle rectangle;
    std::optional<float> baseline = {};
    std::optional<float> centerline = {};

    constexpr box_shape() noexcept = default;
    constexpr box_shape(box_shape const&) noexcept = default;
    constexpr box_shape(box_shape&&) noexcept = default;
    constexpr box_shape& operator=(box_shape const&) noexcept = default;
    constexpr box_shape& operator=(box_shape&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(box_shape const&, box_shape const&) noexcept = default;

    constexpr box_shape(extent2 size) noexcept : rectangle(size), baseline(), centerline() {}

    constexpr box_shape(
        override_t,
        box_constraints const& constraints,
        aarectangle const& rectangle,
        float baseline_adjustment) noexcept :
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

    constexpr box_shape(box_constraints const& constraints, aarectangle rectangle, float baseline_adjustment) noexcept :
        box_shape(override_t{}, constraints, rectangle, baseline_adjustment)
    {
        hi_axiom(rectangle.size() >= constraints.minimum);
    }

    [[nodiscard]] constexpr float x() const noexcept
    {
        return rectangle.x();
    }

    [[nodiscard]] constexpr float y() const noexcept
    {
        return rectangle.y();
    }

    [[nodiscard]] constexpr extent2 size() const noexcept
    {
        return rectangle.size();
    }

    [[nodiscard]] constexpr float width() const noexcept
    {
        return rectangle.width();
    }

    [[nodiscard]] constexpr float height() const noexcept
    {
        return rectangle.height();
    }
};

}} // namespace hi::v1
