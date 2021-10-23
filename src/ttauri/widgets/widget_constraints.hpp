// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/extent.hpp"

namespace tt {

class widget_constraints {
public:
    extent2 min;
    extent2 pref;
    extent2 max;

    constexpr widget_constraints() noexcept : min(), pref(), max() {}
    constexpr widget_constraints(widget_constraints const &) noexcept = default;
    constexpr widget_constraints(widget_constraints &&) noexcept = default;
    constexpr widget_constraints &operator=(widget_constraints const &) noexcept = default;
    constexpr widget_constraints &operator=(widget_constraints &&) noexcept = default;
    constexpr widget_constraints(extent2 min, extent2 pref, extent2 max) noexcept : min(min), pref(pref), max(max)
    {
        tt_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr bool holds_invariant() noexcept
    {
        return min <= pref and pref <= max;
    }
};

} // namespace tt
