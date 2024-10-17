// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "baseline.hpp"
#include "../geometry/geometry.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <limits>
#include <concepts>

hi_export_module(hikogui.layout : box_constraints);

hi_export namespace hi {
inline namespace v1 {

/** 2D constraints.
 * @ingroup geometry
 *
 * This type holds multiple possible sizes that an 2D object may be.
 * We need multiple sizes in case there is a non-linear relation between the width and height of an object.
 *
 */
struct box_constraints {
    extent2 minimum = {};
    extent2 preferred = {};
    extent2 maximum = {};
    hi::margins margins = {};
    hi::baseline baseline = {};

    box_constraints() noexcept = default;
    box_constraints(box_constraints const&) noexcept = default;
    box_constraints(box_constraints&&) noexcept = default;
    box_constraints& operator=(box_constraints const&) noexcept = default;
    box_constraints& operator=(box_constraints&&) noexcept = default;

    box_constraints(
        extent2 minimum,
        extent2 preferred,
        extent2 maximum,
        hi::margins margins = hi::margins{},
        hi::baseline baseline = hi::baseline{}) :
        minimum(minimum), preferred(preferred), maximum(maximum), margins(margins), baseline(baseline)
    {
        assert(holds_invariant());
    }

    box_constraints(
        extent2 size,
        hi::margins margins = hi::margins{},
        hi::baseline baseline = hi::baseline{}) :
        minimum(size), preferred(size), maximum(size), margins(margins), baseline(baseline)
    {
        assert(holds_invariant());
    }

    box_constraints& constrain(extent2 const& new_minimum, extent2 const& new_maximum) noexcept
    {
        assert(new_minimum <= new_maximum);

        minimum = max(minimum, new_minimum);
        maximum = min(maximum, new_maximum);
        preferred = clamp(preferred, minimum, maximum);
        assert(holds_invariant());
        return *this;
    }

    /**
     * Adds an extent to a box_constraints object.
     *
     * This function adds an extent2 object to a box_constraints object and
     * returns a new box_constraints object. The addition is performed by adding
     * the extent values to the minimum, preferred and maximum size values in
     * the box_constraints object. The margins value of the box_constraints
     * object are preserved in the result. The baseline value is reset to zero.
     *
     * @param lhs The box_constraints object to be added to.
     * @param rhs The extent2 object to be added.
     * @return A new box_constraints object resulting from the addition.
     */
    [[nodiscard]] friend box_constraints operator+(box_constraints const& lhs, extent2 const& rhs)
    {
        auto r = box_constraints{};
        r.minimum = lhs.minimum + rhs;
        r.preferred = lhs.preferred + rhs;
        r.maximum = lhs.maximum + rhs;
        r.margins = lhs.margins;
        r.baseline = {};
        assert(r.holds_invariant());
        return r;
    }

    /**
     * Subtracts an extent from a box_constraints object.
     * 
     * This function subtracts an extent2 object to a box_constraints object and
     * returns a new box_constraints object. The operation is performed by
     * subtracting the extent values from the minimum, preferred and maximum
     * size values in the box_constraints object. The margins value of the
     * box_constraints object are preserved in the result. The baseline value is
     * reset to zero.
     *
     * @param lhs The box_constraints object to subtract from.
     * @param rhs The extent to subtract.
     * @return A new box_constraints object with the extent subtracted.
     */
    [[nodiscard]] friend box_constraints operator-(box_constraints const& lhs, extent2 const& rhs)
    {
        auto r = box_constraints{};
        r.minimum = lhs.minimum - rhs;
        r.preferred = lhs.preferred - rhs;
        r.maximum = lhs.maximum - rhs;
        r.margins = lhs.margins;
        r.baseline = {};
        assert(r.holds_invariant());
        return r;
    }

    [[nodiscard]] friend box_constraints operator+(box_constraints const& lhs, hi::margins const& rhs)
    {
        return lhs + rhs.size();
    }

    [[nodiscard]] friend box_constraints operator-(box_constraints const& lhs, hi::margins const& rhs)
    {
        return lhs - rhs.size();
    }

    box_constraints& operator+=(extent2 const& rhs) noexcept
    {
        return *this = *this + rhs;
    }

    box_constraints& operator-=(extent2 const& rhs) noexcept
    {
        return *this = *this - rhs;
    }

    box_constraints& operator+=(hi::margins const& rhs) noexcept
    {
        return *this = *this + rhs;
    }

    box_constraints& operator-=(hi::margins const& rhs) noexcept
    {
        return *this = *this - rhs;
    }

    /**
     * Returns the maximum of two box constraints.
     *
     * @param lhs The first box constraint.
     * @param rhs The second box constraint.
     * @return The maximum box constraint.
     */
    [[nodiscard]] friend box_constraints max(box_constraints const& lhs, box_constraints const& rhs)
    {
        auto r = box_constraints{};
        r.minimum = max(lhs.minimum, rhs.minimum);
        r.preferred = max(lhs.preferred, rhs.preferred);
        r.maximum = max(lhs.maximum, rhs.maximum);
        r.margins = max(lhs.margins, rhs.margins);
        r.baseline = max(lhs.baseline, rhs.baseline);
        assert(r.holds_invariant());
        return r;
    }

    template<std::convertible_to<box_constraints>... Rest>
    [[nodiscard]] friend box_constraints max(box_constraints const& lhs, box_constraints const& rhs, Rest const&... rest)
    {
        return max(lhs, max(rhs, rest...));
    }

    [[nodiscard]] bool holds_invariant() const noexcept
    {
        if (minimum > preferred or preferred > maximum) {
            return false;
        }
        return true;
    }
};

} // namespace v1
} // namespace hi::v1
