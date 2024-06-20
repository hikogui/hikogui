// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "baseline.hpp"
#include "../geometry/geometry.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <limits>
#include <optional>

hi_export_module(hikogui.layout.box_shape);

hi_export namespace hi {
inline namespace v1 {

struct box_shape {
    aarectangle rectangle;
    hi::baseline baseline;

    box_shape() noexcept = default;
    box_shape(box_shape const&) noexcept = default;
    box_shape(box_shape&&) noexcept = default;
    box_shape& operator=(box_shape const&) noexcept = default;
    box_shape& operator=(box_shape&&) noexcept = default;

    box_shape(aarectangle rectangle, hi::baseline baseline) noexcept : rectangle(rectangle), baseline(baseline) {}

    [[nodiscard]] float x() const noexcept
    {
        return rectangle.x();
    }

    [[nodiscard]] float y() const noexcept
    {
        return rectangle.y();
    }

    [[nodiscard]] extent2 size() const noexcept
    {
        return rectangle.size();
    }

    [[nodiscard]] float width() const noexcept
    {
        return rectangle.width();
    }

    [[nodiscard]] float height() const noexcept
    {
        return rectangle.height();
    }
};

} // namespace v1
} // namespace hi::v1
