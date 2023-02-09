
#include <concepts>
#include <limits>

#pragma once

namespace hi {
inline namespace v1 {

template<std::signed_integral T>
struct interval {
    using value_type = T;

    constexpr static value_type min = std::numeric_limits<value_type>::min();
    constexpr static value_type max = std::numeric_limits<value_type>::max();

    value_type lo = lowest;
    value_type hi = heighest;

    constexpr interval() noexcept = default;
    constexpr interval(interval const &) noexcept = default;
    constexpr interval(interval &&) noexcept = default;
    constexpr interval &operator=(interval const &) noexcept = default;
    constexpr interval &operator=(interval &&) noexcept = default;
    [[nodiscard]] constexpr bool operator==(interval const &, interval const &) noexcept = default;

    constexpr interval(value_type lo, value_type hi) noexcept : lo(lo), hi(hi)
    {
        hi_axiom(lo <= hi);
    }

    constexpr interval(value_type rhs) noexcept : lo(rhs), hi(rhs)
    {
    }

    /** The interval is finite,
     */
    [[nodiscard]] constexpr bool finite() const noexcept
    {
        return lo != min and hi != max;
    }

    constexpr interval &operator++() noexcept
    {
        lo = lo == min ? min : saturated_inc(lo);
        hi = saturated_inc(hi);
        return *this;
    }

    constexpr interval &operator--() noexcept
    {
        lo = saturated_dec(lo);
        hi = hi == max ? max : saturated_dec(hi);
        return *this;
    }

    constexpr interval operator++(int) noexcept
    {
        auto r = *this;
        ++(*this);
        return r;
    }

    constexpr interval operator--(int) noexcept
    {
        auto r = *this;
        --(*this);
        return r;
    }

    constexpr interval &operator+=(interval const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    constexpr interval &operator-=(interval const &rhs) noexcept
    {
        return *this = *this - rhs;
    }

    [[nodiscard]] constexpr friend interval operator-(interval const &rhs) noexcept
    {
        hilet r_lo = lhs.hi == max ? min : saturated_neg(lhs.hi);
        hilet r_hi = lhs.lo == min ? max : saturated_neg(lhs.lo);
        return {r_lo, r_hi};
    }

    [[nodiscard]] constexpr friend interval operator+(interval const &lhs, interval const &rhs) noexcept
    {
        hilet r_lo = lhs.lo == min or rhs.lo == min ? min : saturated_add(lhs.lo, rhs.lo);
        hilet r_hi = lhs.hi == max or rhs.hi == max ? max : saturated_add(lhs.hi, rhs.hi);
        return {r_lo, r_hi};
    }

    [[nodiscard]] constexpr friend interval operator-(interval const &lhs, interval const &rhs) noexcept
    {
        hilet r_lo = lhs.lo == min or rhs.hi == min ? max : saturated_sub(lhs.lo, rhs.hi);
        hilet r_hi = lhs.hi == max or rhs.lo == min ? max : saturated_sub(lhs.hi, rhs.lo);
        return {r_lo, r_hi};
    }

    [[nodiscard]] constexpr friend interval abs(interval const &rhs) noexcept
    {
        auto r_lo = saturated_abs(rhs.lo);
        auto r_hi = saturated_abs(rhs.hi);
        if (r_lo > r_hi) {
            std::swap(r_lo, r_hi);
        }
        return {r_lo, r_hi};
    }

    [[nodiscard]] constexpr friend interval min(interval const &lhs,interval const &rhs) noexcept
    {
        auto r_lo = std::min(lhs.lo, rhs.lo);
        auto r_hi = lhs.hi == max or rhs.hi == max ? max : std::min(lhs.hi, rhs.hi);
        return {r_lo, r_hi};
    }

    [[nodiscard]] constexpr friend interval max(interval const &lhs,interval const &rhs) noexcept
    {
        auto r_lo = lhs.lo == min or rhs.lo == min ? min : std::max(lhs.lo, rhs.lo);
        auto r_hi = std::max(lhs.hi, rhs.hi);
        return {r_lo, r_hi};
    }
};

}}
