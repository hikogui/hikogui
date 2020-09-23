// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <type_traits>
#include <limits>
#include <concepts>

namespace tt {

template<typename T>
requires std::floating_point<T> class interval {
public:
    using value_type = T;

    constexpr interval() noexcept :
        _minimum(std::numeric_limits<value_type>::max()), _maximum(std::numeric_limits<value_type>::max())
    {
    }

    constexpr interval(interval const &rhs) noexcept = default;
    constexpr interval(interval &&rhs) noexcept = default;
    constexpr interval &operator=(interval const &rhs) noexcept = default;
    constexpr interval &operator=(interval &&rhs) noexcept = default;

    [[nodiscard]] constexpr interval(value_type _minimum, value_type _maximum) noexcept : _minimum(-_minimum), _maximum(_maximum) {
        tt_assume(minimum() <= maximum());
    }

    [[nodiscard]] constexpr interval(value_type rhs) noexcept : interval(rhs, rhs) {}

    [[nodiscard]] constexpr value_type minimum() const noexcept
    {
        return -_minimum;
    }

    [[nodiscard]] constexpr value_type maximum() const noexcept
    {
        return _maximum;
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
        tt_assume(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval operator-(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        for (int i = 0; i != 2; ++i) {
            r.v[i] = lhs.v[i] - rhs.v[1 - i];
        }
        tt_assume(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval max(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        r._minimum = lhs._minimum < rhs._minimum ? lhs._minimum : rhs._minimum; // std::min()
        r._maximum = lhs._maximum > rhs._maximum ? lhs._maximum : rhs._maximum; // std::max()
        tt_assume(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval min(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        r._minimum = lhs._minimum > rhs._minimum ? lhs._minimum : rhs._minimum; // std::max()
        r._maximum = lhs._maximum < rhs._maximum ? lhs._maximum : rhs._maximum; // std::min()
        tt_assume(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval intersect(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        for (int i = 0; i != 2; ++i) {
            r.v[i] = lhs.v[i] < rhs.v[i] ? lhs.v[i] : rhs.v[i]; // std::min()
        }
        tt_assume(r.minimum() <= r.maximum());
        return r;
    }

    [[nodiscard]] friend constexpr interval merge(interval const &lhs, interval const &rhs) noexcept
    {
        auto r = interval{};
        for (int i = 0; i != 2; ++i) {
            r.v[i] = lhs.v[i] > rhs.v[i] ? lhs.v[i] : rhs.v[i]; // std::max()
        }
        tt_assume(r.minimum() <= r.maximum());
        return r;
    }

private:
    union {
        alignas(sizeof(value_type) * 2) value_type v[2];
        struct {
            value_type _minimum;
            value_type _maximum;
        };
    };
};

using finterval = interval<float>;
using dinterval = interval<double>;

} // namespace tt