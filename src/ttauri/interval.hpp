// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include "concepts.hpp"
#include "rapid/numeric_array.hpp"
#include <type_traits>
#include <limits>
#include <concepts>
#include <algorithm>
#include <compare>

namespace tt {

/** Interval arithmetic.
 *
 * Based on: "INTERVAL ARITHMETIC USING SSE-2 (DRAFT)" - BRANIMIR LAMBOV
 * 
 * This interval is implemented with a negated upper bound, this
 * allows the rounding direction to negative infinity.
 *
 * @tparam T A type for which std::numeric_limits<T> is implemented.
 */
template<numeric_limited T>
struct interval {
public:
    using value_type = T;
    using bound_type = numeric_array<value_type,2>;

    bound_type v;

    constexpr interval(interval const &rhs) noexcept = default;
    constexpr interval(interval &&rhs) noexcept = default;
    constexpr interval &operator=(interval const &rhs) noexcept = default;
    constexpr interval &operator=(interval &&rhs) noexcept = default;

    /** Default construct an interval.
    * 
    * The interval includes all values of the value_type.
    */
    constexpr interval() noexcept
    {
        if constexpr (std::is_floating_point_v<value_type>) {
            v[0] = -std::numeric_limits<value_type>::infinite();
            v[1] = -std::numeric_limits<value_type>::infinite();
        } else {
            v[0] = std::numeric_limits<value_type>::min();
            v[1] = -std::numeric_limits<value_type>::max();
        }
        tt_axiom(holds_invariant());
    }

    /** Construct an interval from a lower and upper bounds.
    * 
    * @param lower The lower bound.
    * @param upper The upper bound.
    */
    [[nodiscard]] constexpr interval(value_type lower, value_type upper) noexcept : 
        v(lower, -upper)
    {
        tt_axiom(holds_invariant());
    }

    /** Construct an interval from a bound_type value.
    * 
    * @param bounds The bounds x=lower, y=-upper.
    * @return An interval.
    */
    [[nodiscard]] static constexpr interval raw(bound_type bounds) noexcept
    {
        interval r;
        r.v = bounds;
        tt_axiom(r.holds_invariant());
        return r;
    }
    
    [[nodiscard]] bool constexpr holds_invariant() const noexcept
    {
        return -lower <= upper;
    }

    /** Get the lower bound of the interval.
    * 
    * @return The lower bound.
    */
    [[nodiscard]] constexpr value_type lower() const noexcept
    {
        return v[0];
    }

    /** Get the upper bound of the interval.
     *
     * @return The upper bound.
     */
    [[nodiscard]] constexpr value_type upper() const noexcept
    {
        return -v[1];
    }

    /** The distance between lower and upper bound.
    * 
    * @return the difference between the lower and upper bound. Must be always larger or equal to zero.
     */
    [[nodiscard]] constexpr value_type delta() const noexcept
    {
        return upper() - lower();
    }

    /** Check if the interval is one value.
    * 
    * @return true if the delta is zero.
    */
    [[nodiscard]] constexpr bool is_value() const noexcept
    {
        return delta() == 0;
    }

    /** Check if the interval is a range of values.
    * 
    * @return true if the delta is greater than zero.
    */
    [[nodiscard]] constexpr bool is_range() const noexcept
    {
        return delta() > 0;
    }

    /** Check if the interval is true.
    * 
    * @return false if both the lower and upper bound are zero.
    */
    operator bool () const noexcept
    {
        return v[0] != 0 or v[1] != 0;
    }

    [[nodiscard]] constexpr interval operator-() const noexcept
    {
        return interval::raw(v.yx());
    }

    [[nodiscard]] constexpr interval operator+(interval const &rhs) const noexcept
    {
        return interval::raw(v + rhs.v);
    }

    [[nodiscard]] constexpr interval operator-(interval const &rhs) const noexcept
    {
        return *this + (-rhs);
    }

    [[nodiscard]] constexpr interval operator*(interval const &rhs) const noexcept
    {
        ttlet ge_zero = ge(v, bound_type{});
        ttlet lt_zero = ~ge_zero;

        ttlet ac = (ge_zero & rhs.v.xx()) | (lt_zero & -rhs.v.yy());
        ttlet db = (ge_zero & rhs.v.yy()) | (lt_zero & -rhs.v.xx());

        ttlet ac_mul = ac * v;
        ttlet db_mul = db * v;

        return raw(min(ac_mul, db_mul.yx()));
    }

    /** Multiply two positive intervals.
     */
    [[nodiscard]] constexpr interval positive_mul(interval const &rhs) const noexcept
    {
        tt_axiom(v[0] >= 0 and rhs.v[0] >= 0);

        return raw(v * neg<0b10>(rhs.v));
    }

    [[nodiscard]] constexpr interval operator/(interval const &rhs) const noexcept
    {
        if (rhs.v[0] <= 0 and 0 <= rhs.v[1]) {
            // Return an infinite interval when it is possible to divide by zero.
            return interval{};
        }

        ttlet rhs_ge_zero = ge(rhs.v, bound_type{});
        ttlet rhs_lt_zero = ~rhs_ge_zero;

        ttlet b_ma = (rhs_ge_zero & v.yy()) | (rhs_lt_zero & -v.xx());
        ttlet a_mb = -b_ma.yx();

        ttlet a_mb_mul = a_mb / rhs.v;
        ttlet b_ma_mul = b_ma / rhs.v;

        return raw(min(a_mb, b_ma));
    }

    [[nodiscard]] friend constexpr interval reciprocal(interval const &rhs) noexcept
    {
        if (rhs.v[0] <= 0 and 0 <= rhs.v[1]) {
            // Return an infinite interval when it is possible to divide by zero.
            return interval{};
        }

        return raw(bound_type::broadcast(value_type{-1}) / rhs.yx());
    }

    [[nodiscard]] friend constexpr interval abs(interval const &rhs) noexcept
    {
        bound_type r;
        r[0] = std::max({value_type{0}, rhs.v[0], rhs.v[1]});
        r[1] = std::min(rhs.v[0], rhs.v[1]);
        return raw(r);
    }

    [[nodiscard]] friend constexpr interval square(interval const &rhs) noexcept
    {
        ttlet abs_rhs = abs(rhs);
        return abs_rhs.positive_mul(abs_rhs);
    }

    interval &operator+=(interval const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    interval &operator-=(interval const &rhs) noexcept
    {
        return *this = *this - rhs;
    }

    interval &operator*=(interval const &rhs) noexcept
    {
        return *this = *this * rhs;
    }

    [[nodiscard]] friend constexpr bool operator==(value_type const &lhs, interval const &rhs) noexcept
    {
        return rhs.lower() <= lhs and lhs <= rhs.upper();
    }

    [[nodiscard]] friend constexpr auto<=>(value_type const &lhs, interval const &rhs) noexcept
    {
        if constexpr (std::is_floating_point_v<value_type>) {
            if (lhs < rhs.lower()) {
                return std::partial_ordering::less;
            } else if (lhs > rhs.upper()) {
                return std::partial_ordering::greater;
            } else if (lhs >= rhs.lower() and lhs <= rhs.upper()) {
                return std::partial_ordering::equivalent;
            } else {
                return std::partial_ordering::unordered;
            }
        } else {
            if (lhs < rhs.lower()) {
                return std::weak_ordering::less;
            } else if (lhs > rhs.upper()) {
                return std::weak_ordering::greater;
            } else {
                return std::weak_ordering::equivalent;
            }
        }
    }

    /** Check if the current interval is fully inside the other interval.
    * 
    * @param other The other interval.
    * @return true If this interval is fully inside the @a other interval.
    */
    [[nodiscard]] constexpr bool is_fully_inside(interval const &other) const noexcept
    {
        return ge(v, other.v) == 0b11;
    }

};

using finterval = interval<float>;
using dinterval = interval<double>;

} // namespace tt
