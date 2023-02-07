
#include <concepts>
#include <limits>

#pragma once

namespace hi {
inline namespace v1 {

template<std::signed_integral T>
struct interval {
    using value_type = T;

    constexpr static value_type lowest = std::numeric_limits<value_type>::min();
    constexpr static value_type heighest = std::numeric_limits<value_type>::max();

    saturated<value_type> lo = lowest;
    saturated<value_type> hi = heighest;

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


    [[nodiscard]] constexpr bool lo_saturated() const noexcept
    {
        return lo == lowest;
    }

    [[nodiscard]] constexpr bool hi_saturated() const noexcept
    {
        return hi == heighest;
    }

    [[nodiscard]] constexpr friend interval operator+(interval const &lhs, interval const &rhs) noexcept
    {
        hilet new_lo = (lhs.lo_saturated() or rhs.lo_saturated()) ? lowest : lhs.lo + rhs.lo;
        hilet new_hi = (lhs.hi_saturated() or rhs.hi_saturated()) ? heighest : lhs.hi + rhs.hi;
        return {new_lo, new_hi};
    }

    [[nodiscard]] constexpr friend interval operator-(interval const &lhs, interval const &rhs) noexcept
    {
        hilet new_lo = (lhs.lo_saturated() or rhs.hi_saturated()) ? lowest : lhs.lo - rhs.hi;
        hilet new_hi = (lhs.hi_saturated() or rhs.lo_saturated()) ? heighest : lhs.hi - rhs.lo;
        return {new_lo, new_hi};
    }

    [[nodiscard]] constexpr friend interval operator*(interval const &lhs, interval const &rhs) noexcept
    {
        return {new_lo, new_hi};
    }
};

}}
