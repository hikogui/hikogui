// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include "concepts.hpp"
#include <type_traits>
#include <limits>
#include <concepts>
#include <algorithm>

namespace tt {

/** Interval arithmatic.
 *
 * This interval is implemented with a negated upper bound, this
 * allows the rounding direction to negative infinity.
 *
 * @tparam T numeric type
 */
template<numeric_limited T>
struct interval {
public:
    using value_type = T;

    value_type lower;
    value_type upper;

    constexpr interval() noexcept : v()
    {
        lower = std::numeric_limits<value_type>::min();
        upper = -std::numeric_limits<value_type>::max();
    }

    constexpr interval(interval const &rhs) noexcept = default;
    constexpr interval(interval &&rhs) noexcept = default;
    constexpr interval &operator=(interval const &rhs) noexcept = default;
    constexpr interval &operator=(interval &&rhs) noexcept = default;

    [[nodiscard]] constexpr interval(value_type lower, value_type upper) noexcept : 
        lower(lower), upper(-upper)
    {
        tt_axiom(holds_invariant());
    }

    [[nodiscard]] interval static constexpr raw(value_type lower, value_type upper) noexcept
    {
        interval r;
        r.lower = lower;
        r.upper = upper;
        tt_axiom(r.holds_invariant());
        return r;
    }

    [[nodiscard]] constexpr interval(value_type rhs) noexcept : interval(rhs, rhs) {}

    [[nodiscard]] constexpr interval operator-() const noexcept
    {
        return raw(upper, lower);
    }

    [[nodiscard]] constexpr interval operator+(interval const &rhs) const noexcept
    {
        return raw(lower + rhs.lower, upper + rhs.upper);
    }

    [[nodiscard]] constexpr interval operator-(interval const &rhs) const noexcept
    {
        return *this + (-rhs);
    }

    [[nodiscard]] constexpr interval operator*(interval const &rhs) const noexcept
    {
        ttlet a = lower >= 0 ? rhs.lower : -rhs.upper;
        ttlet b = upper >= 0 ? -rhs.lower : rhs.upper;
        ttlet c = upper >= 0 ? -rhs.upper : rhs.lower;
        ttlet d = lower >= 0 ? rhs.upper : -rhs.lower;
        ttlet al = a * lower;
        ttlet bu = b * upper;
        ttlet cu = c * upper;
        ttlet dl = d * lower;
        ttlet al_bu = al < bu ? al : bu;
        ttlet cu_dl = cu < dl ? cu : dl;
        return raw(al_bu, cu_dl);
    }

    [[nodiscard]] constexpr interval operator/(interval const &rhs) const noexcept
    {
         ttlet a = rhs.upper <= 0 ? lower, -upper;
         ttlet b = rhs.lower >= 0 ? upper, -lower;
         ttlet al = a / rhs.lower;
         ttlet au = -a / rhs.upper;
         ttlet bu = -b / rhs.upper;
         ttlet bl = b / rhs.lower;
         ttlet al_au = al < au ? al : au;
         ttlet bu_bl = bu < bl ? bu : bl;
         return raw(al_au, bu_bl);
    }

    interval &operator+=(interval const &rhs) noexcept
    {
        lower += rhs.lower;
        upper += rhs.upper;
        tt_axiom(holds_invariant());
        return *this;
    }

    interval &operator-=(interval const &rhs) noexcept
    {
        *this += -rhs;
        tt_axiom(holds_invariant());
        return *this;
    }

    interval &operator*=(interval const &rhs) noexcept
    {
        *this = *this * rhs;
        tt_axiom(holds_invariant());
        return *this;
    }

    /** lhs is less than the upper edge of the interval.
     */
    [[nodiscard]] friend constexpr bool operator<(value_type const &lhs, interval const &rhs) noexcept
    {
        return lhs < rhs.maximum();
    }

    /** lhs is less than or equal the upper edge of the interval.
     */
    [[nodiscard]] friend constexpr bool operator<=(value_type const &lhs, interval const &rhs) noexcept
    {
        return lhs <= rhs.maximum();
    }

    /** lhs is less than and outside of the interval
     */
    [[nodiscard]] friend constexpr bool operator<<(value_type const &lhs, interval const &rhs) noexcept
    {
        return lhs < rhs.minimum();
    }

    /** lhs is greater than the lower edge of the interval.
     */
    [[nodiscard]] friend constexpr bool operator>(value_type const &lhs, interval const &rhs) noexcept
    {
        return lhs > rhs.minimum();
    }

    /** lhs is greater than or equal to the lower edge of the interval.
     */
    [[nodiscard]] friend constexpr bool operator>=(value_type const &lhs, interval const &rhs) noexcept
    {
        return lhs >= rhs.minimum();
    }

    /** lhs is greater than and outside of the interval
     */
    [[nodiscard]] friend constexpr bool operator>>(value_type const &lhs, interval const &rhs) noexcept
    {
        return lhs > rhs.maximum();
    }

    [[nodiscard]] friend constexpr interval max(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        r.v[0] = lhs.v[0] < rhs.v[0] ? lhs.v[0] : rhs.v[0]; // std::min()
        r.v[1] = lhs.v[1] > rhs.v[1] ? lhs.v[1] : rhs.v[1]; // std::max()
        tt_axiom(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval min(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        r.v[0] = lhs.v[0] > rhs.v[0] ? lhs.v[0] : rhs.v[0]; // std::max()
        r.v[1] = lhs.v[1] < rhs.v[1] ? lhs.v[1] : rhs.v[1]; // std::min()
        tt_axiom(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval intersect(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        for (int i = 0; i != 2; ++i) {
            r.v[i] = lhs.v[i] < rhs.v[i] ? lhs.v[i] : rhs.v[i]; // std::min()
        }
        tt_axiom(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval merge(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        for (int i = 0; i != 2; ++i) {
            r.v[i] = lhs.v[i] > rhs.v[i] ? lhs.v[i] : rhs.v[i]; // std::max()
        }
        tt_axiom(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr value_type clamp(value_type const &lhs, interval const &rhs) noexcept
    {
        tt_axiom(rhs.minimum() <= rhs.maximum());
        return std::clamp(lhs, rhs.minimum(), rhs.maximum());
    }

    [[nodiscard]] bool constexpr holds_invariant() const noexcept
    {
        return -lower <= upper;
    }

};

using finterval = interval<float>;
using dinterval = interval<double>;

} // namespace tt
