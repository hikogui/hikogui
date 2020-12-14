// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include <type_traits>
#include <limits>
#include <concepts>
#include <algorithm>

namespace tt {

template<typename T>
requires std::floating_point<T> class interval {
public:
    using value_type = T;

    constexpr interval() noexcept : v()
    {
        v[0] = std::numeric_limits<value_type>::max();
        v[1] = std::numeric_limits<value_type>::max();
    }

    constexpr interval(interval const &rhs) noexcept = default;
    constexpr interval(interval &&rhs) noexcept = default;
    constexpr interval &operator=(interval const &rhs) noexcept = default;
    constexpr interval &operator=(interval &&rhs) noexcept = default;

    [[nodiscard]] constexpr interval(value_type _minimum, value_type _maximum) noexcept : v()
    {
        v[0] = -_minimum;
        v[1] = _maximum;
        tt_axiom(minimum() <= maximum());
    }

    [[nodiscard]] constexpr interval(value_type rhs) noexcept : interval(rhs, rhs) {}

    [[nodiscard]] constexpr value_type minimum() const noexcept
    {
        return -v[0];
    }

    [[nodiscard]] constexpr value_type maximum() const noexcept
    {
        return v[1];
    }

    interval &operator+=(interval const &rhs) noexcept
    {
        for (int i = 0; i != 2; ++i) {
            v[i] += rhs.v[i];
        }
        return *this;
    }

    interval &operator-=(interval const &rhs) noexcept
    {
        for (int i = 0; i != 2; ++i) {
            v[i] -= rhs.v[1 - i];
        }
        return *this;
    }

    [[nodiscard]] friend constexpr interval operator+(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        for (int i = 0; i != 2; ++i) {
            r.v[i] = lhs.v[i] + rhs.v[i];
        }
        tt_axiom(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval operator-(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        for (int i = 0; i != 2; ++i) {
            r.v[i] = lhs.v[i] - rhs.v[1 - i];
        }
        tt_axiom(r.minimum() <= r.maximum());
        return r;
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

private:
    alignas(sizeof(value_type) * 2) value_type v[2];
};

using finterval = interval<float>;
using dinterval = interval<double>;

} // namespace tt