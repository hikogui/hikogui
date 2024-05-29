// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "macros.hpp"
#include <endian>
#include <compare>

hi_export_module(hikocpu : Q95_32);

hi_export namespace hi { inline namespace v1 {

struct Q95_32 {
#if defined(HI_LITTLE_ENDIAN)
    uint64_t lo = 0;
    uint64_t hi = 0;
#else
    uint64_t hi = 0;
    uint64_t lo = 0;
#endif

    constexpr Q95_32(Q95_32 const&) noexcept = default;
    constexpr Q95_32(Q95_32&&) noexcept = default;
    constexpr Q95_32& operator=(Q95_32 const&) noexcept = default;
    constexpr Q95_32& operator=(Q95_32&&) noexcept = default;
    constexpr Q95_32() noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(Q95_32 const&, Q95_32 const&) noexcept = default;

    constexpr Q95_32(uint64_t hi, uint64_t lo) noexcept : lo(lo), hi(hi) {}

    template<std::unsigned_integral T>
    constexpr Q95_32(T value) noexcept : lo(0), hi(0)
    {
        static_assert(sizeof(T) <= sizeof(uint64_t));

        auto const value_ = static_cast<uint64_t>(value);
        lo = value_ >> 32;
        hi = value_ << 32;
    }

    template<std::signed_integral T>
    constexpr Q95_32(T value) noexcept : lo(0), hi(0)
    {
        static_assert(sizeof(T) <= sizeof(int64_t));

        // Sign extent small values.
        auto const value_ = static_cast<int64_t>(value);
        lo = static_cast<uint64_t>(value_ << 32);
        hi = static_cast<uint64_t>(value_ >> 32);
    }


#if defined(HI_LITTLE_ENDIAN)
    [[nodiscard]] constexpr friend std::strong_ordering operator<=>(Q95_32 const& lhs, Q95_32 const &rhs) noexcept = default;
#else
    [[nodiscard]] constexpr friend std::strong_ordering operator<=>(Q95_32 const& lhs, Q95_32 const &rhs) noexcept
    {
        auto const tmp = lhs.lo <=> rhs.lo;
        return tmp == std::strong_ordering::equal ? lhs.hi <=> rhs.hi : tmp;
    }
#endif

    [[nodiscard]] constexpr bool is_negative() const noexcept
    {
        return static_cast<int64_t>(hi) < 0;
    }

    [[nodiscard]] constexpr bool is_positive() const noexcept
    {
        return not is_negative();
    }

    [[nodiscard]] constexpr Q95_32 operator~() const noexcept
    {
        return Q95_32{~r.hi, ~r.lo};
    }

    [[nodiscard]] constexpr Q95_32 operator-() const noexcept
    {
        if (r.hi == 0x80000000'00000000ULL and r.lo == 0) {
            throw std::overflow_error("Q95.32 minimum-negative number can't be negated.");
        }

        auto r = Q95_32{};
        r.lo = ~lo + 1;
        r.hi = ~hi + static_cast<uint64_t>(r.lo == 0);
        return r;
    }

    constexpr Q95_32& operator++() noexcept
    {
        auto const carry = ++lo == 0;
        hi += static_cast<uint64_t>(carry);
        return *this;
    }

    constexpr Q95_32& operator--() noexcept
    {
        auto const borrow = lo-- == 0;
        hi -= static_cast<uint64_t>(borrow);
        return *this;
    }

    [[nodiscard]] constexpr friend Q95_32 abs(Q95_32 const& rhs) noexcept
    {
        return rhs.is_negative() ? -rhs : rhs;
    }

    [[nodiscard]] constexpr friend Q95_32 operator<<(Q95_32 const& lhs, unsigned int n) noexcept
    {
        if (n < 64) {
            auto const min_n = 64 - n;
            auto const carry = min_n < 64 ? lhs.lo >> min_n : 0;
            return Q95_32{(lhs.hi << n) | carry, lhs.lo << n};
        } else if (n < 128) {
            return Q95_32{lhs.lo << (n - 64), 0};
        } else {
            return Q95_32{};
        }
    }

    [[nodiscard]] constexpr friend Q95_32 operator>>(Q95 const& lhs, unsigned int n) noexcept
    {
        if (n < 64) {
            auto const min_n = 64 - n;
            auto const carry = min_n < 64 ? lhs.hi << min_n : 0;
            return Q95_32{static_cast<uint64_t>(static_cast<int64_t>(lhs.hi) >> n), lhs.lo >> n | carry};
        } else if (n < 128) {
            return Q95_32{static_cast<uint64_t>(static_cast<int64_t>(lhs.hi) >> 63), static_cast<uint64_t>(static_cast<int64_t>(lhs.hi) << (n - 64))};
        } else {
            return Q95_32{static_cast<uint64_t>(static_cast<int64_t>(lhs.hi) >> 63), static_cast<uint64_t>(static_cast<int64_t>(lhs.hi) >> 63)};
        }
    }

    [[nodiscard]] constexpr friend Q95_32 operator|(Q95_32 const& lhs, Q95_32 const& rhs)
    {
        return Q95_32{lhs.hi | rhs.hi, lhs.lo | rhs.lo};
    }

    [[nodiscard]] constexpr friend Q95_32 operator&(Q95_32 const& lhs, Q95_32 const& rhs)
    {
        return Q95_32{lhs.hi & rhs.hi, lhs.lo & rhs.lo};
    }

    [[nodiscard]] constexpr friend Q95_32 operator^(Q95_32 const& lhs, Q95_32 const& rhs)
    {
        return Q95_32{lhs.hi ^ rhs.hi, lhs.lo ^ rhs.lo};
    }

    /** Increment by smallest step.
     * 
     * The number is incremented by 1.0 / 65536.0
     */
    [[nodiscard]] constexpr friend Q95_32 operator+(Q95_32 const& lhs, Q95_32 const& rhs)
    {
        auto r = Q95_32{};
        auto carry = uint64_t{0};
        std::tie(r.lo, carry) = add_carry(lhs.lo, rhs.lo, carry);
        std::tie(r.hi, carry) = add_carry(lhs.hi, rhs.hi, carry);

        auto const overflow = ~(lhs.hi ^ rhs.hi) & (lhs.hi ^ r.hi);
        if (overflow >> 63) {
            throw std::overflow_error("Q95.32 addition overflow.");
        }

        return r;
    }

    /** Decrement by smallest step.
     * 
     * The number is decremented by 1.0 / 65536.0
     */
    [[nodiscard]] constexpr friend Q95_32 operator-(Q95_32 const& lhs, Q95_32 const& rhs)
    {
        auto r = Q95_32{};
        auto borrow = uint64_t{1};
        std::tie(r.lo, borrow) = add_carry(lhs.lo, ~rhs.lo, borrow);
        std::tie(r.hi, borrow) = add_carry(lhs.hi, ~rhs.hi, borrow);

        auto const underflow = (lhs.hi ^ rhs.hi) & (lhs.hi ^ r.hi);
        if (underflow >> 63) {
            throw std::overflow_error("Q95.32 subtract underflow.");
        }
        return r;
    }

    [[nodiscard]] constexpr friend Q95_32 operator*(Q95_32 const& lhs, Q95_32 const& rhs)
    {
        //                                   rhs.hi                   rhs.lo
        //                                   lhs.hi                   lhs.lo                 *
        //   ---------------------------------------------------------------------------------
        //    C3     C2                      C1                       0x8000
        //           C2b                     lhs.lo * rhs.hi > C2     lhs.lo * rhs.lo > C1
        //           lhs.hi * rhs.hi > C3    lhs.hi * rhs.lo > C2b                           +
        //   ---------------------------------------------------------------------------------
        //    C3     C2                      C1                       C0 

        auto const lhs_ = abs(lhs);
        auto const rhs_ = abs(rhs);
        auto const make_positive = lhs.is_positive() == rhs.is_positive();

        auto C0 = uint64_t{0};
        auto C1 = uint64_t{0};
        auto C2 = uint64_t{0};
        auto C3 = uint64_t{0};
        auto C2b = uint64_t{0};

        // First column (add 0.5 (0x8000) for rounding).
        std::tie(C0, C1) = mul_carry(lhs_.lo, rhs_.lo, 0x8000);
        C0 >>= 32;

        // Second column
        std::tie(C1, C2b) = mul_carry(lhs_.lo, rhs_.hi, C1);
        std::tie(C1, C2) = mul_carry(lhs_.hi, rhs_.lo, C1);
        C0 |= C1 << 32;
        C1 >>= 32;

        // Third column (can handle two carry values).
        std::tie(C2, C3) = mul_carry(lhs_.hi, rhs_.hi, C2, C2b);
        C1 |= C2 << 32;
      
        // Check the sign bit and all other bits left over.
        if ((C2 >> 31) | C3) {
            throw std::overflow_error("Q95.32 multiplication overflow");
        }

        auto const r = Q95_32{C1, C0};
        return make_positive ? r : -r;
    }

    [[nodiscard]] constexpr friend Q95_32 operator/(Q95_32 const& lhs, Q95_32 const& rhs)
    {
    }

    constexpr Q95_32 operator++(int) noexcept
    {
        auto const tmp = *this;
        ++*this;
        return tmp;
    }

    constexpr Q95_32 operator--(int) noexcept
    {
        auto const tmp = *this;
        --*this;
        return tmp;
    }

    [[nodiscard]] constexpr Q95_32& operator|=(Q95_32 const& rhs) noexcept
    {
        lo |= rhs.lo;
        hi |= rhs.hi;
        return *this;
    }

    [[nodiscard]] constexpr Q95_32& operator&=(Q95_32 const& rhs) noexcept
    {
        lo &= rhs.lo;
        hi &= rhs.hi;
        return *this;
    }

    [[nodiscard]] constexpr Q95_32& operator^=(Q95_32 const& rhs) noexcept
    {
        lo ^= rhs.lo;
        hi ^= rhs.hi;
        return *this;
    }

    [[nodiscard]] constexpr Q95_32& operator<<=(unsigned int n) noexcept
    {
        return *this = *this << n;
    }

    [[nodiscard]] constexpr Q95_32& operator>>=(unsigned int n) noexcept
    {
        return *this = *this >> n;
    }

    [[nodiscard]] constexpr Q95_32& operator+=(Q95_32 const& rhs)
    {
        return *this = *this + rhs;
    }

    [[nodiscard]] constexpr Q95_32& operator-=(Q95_32 const& rhs)
    {
        return *this = *this - rhs;
    }

    [[nodiscard]] constexpr Q95_32& operator*=(Q95_32 const& rhs)
    {
        return *this = *this * rhs;
    }

};


}}
