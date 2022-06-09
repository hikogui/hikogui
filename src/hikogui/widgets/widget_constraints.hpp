// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/extent.hpp"
#include "../geometry/margins.hpp"
#include "widget_baseline.hpp"

namespace hi::inline v1 {

class widget_constraints {
public:
    extent2 minimum;
    extent2 preferred;
    extent2 maximum;
    margins margins;
    widget_baseline baseline;

    constexpr widget_constraints() noexcept : minimum(), preferred(), maximum(), margins(), baseline() {}
    constexpr widget_constraints(widget_constraints const&) noexcept = default;
    constexpr widget_constraints(widget_constraints&&) noexcept = default;
    constexpr widget_constraints& operator=(widget_constraints const&) noexcept = default;
    constexpr widget_constraints& operator=(widget_constraints&&) noexcept = default;
    constexpr widget_constraints(
        extent2 minimum,
        extent2 preferred,
        extent2 maximum,
        hi::margins margins = hi::margins{},
        widget_baseline baseline = hi::widget_baseline{}) noexcept :
        minimum(minimum), preferred(preferred), maximum(maximum), margins(margins), baseline(baseline)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr bool holds_invariant() noexcept
    {
        return minimum <= preferred and preferred <= maximum;
    }

    [[nodiscard]] friend constexpr widget_constraints operator+(widget_constraints const& lhs, extent2 const& rhs) noexcept
    {
        return widget_constraints{lhs.minimum + rhs, lhs.preferred + rhs, lhs.maximum + rhs, lhs.margins, lhs.baseline};
    }

    [[nodiscard]] friend constexpr bool
    operator==(widget_constraints const& lhs, widget_constraints const& rhs) noexcept = default;

    [[nodiscard]] friend constexpr widget_constraints max(widget_constraints const& lhs, widget_constraints const& rhs) noexcept
    {
        return widget_constraints{
            max(lhs.minimum, rhs.minimum),
            max(lhs.preferred, rhs.preferred),
            max(lhs.maximum, rhs.maximum),
            max(lhs.margins, rhs.margins),
            max(lhs.baseline, rhs.baseline)};
    }

    [[nodiscard]] friend constexpr widget_constraints max(widget_constraints const& lhs, extent2 const& rhs) noexcept
    {
        return widget_constraints{max(lhs.minimum, rhs), max(lhs.preferred, rhs), max(lhs.maximum, rhs), lhs.margins, lhs.baseline};
    }

    template<typename... Args>
    [[nodiscard]] friend constexpr widget_constraints max(widget_constraints const& first, Args const&...args) noexcept
        requires(sizeof...(Args) >= 2)
    {
        return max(first, max(args...));
    }
};

} // namespace hi::inline v1
