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
    uint64_t lo;
    uint64_t hi;
#else
    uint64_t hi;
    uint64_t lo;
#endif

    constexpr Q95_32(Q95_32 const&) noexcept = default;
    constexpr Q95_32(Q95_32&&) noexcept = default;
    constexpr Q95_32& operator=(Q95_32 const&) noexcept = default;
    constexpr Q95_32& operator=(Q95_32&&) noexcept = default;

    constexpr Q95_32() noexcept : lo(0), hi(0) {}

    template<std::unsigned_integral T>
    constexpr Q95_32(T value) noexcept : lo(0), hi(0)
    {
        if constexpr (sizeof(T) <= 4) {
            lo = static_cast<uint64_t>(value) << 32;

        } else {
            lo = static_cast<uint64_t>(value) << 32;
            hi = static_cast<uint64_t>(value) >> (sizeof(T) * CHAR_BIT - 32);
        }
    }

    template<std::signed_integral T>
    constexpr Q95_32(T value) noexcept : lo(0), hi(0)
    {
        if constexpr (sizeof(T) <= 4) {
            auto value_ = static_cast<int64_t>(static_cast<uint64_t>(value));
            _value <<= 64 - sizeof(T) * CHAR_BIT;
            _value >>= 32 - sizeof(T) * CHAR_BIT;
            lo = static_cast<uint64_t>(_value);
            _value >>= 63;
            hi = static_cast<uint64_t>(_value);

        } else {
            lo = static_cast<uint64_t>(value) << 32;
            hi = static_cast<uint64_t>(value) >> ((sizeof(T) * CHAR_BIT) - 32);
        }
    }

    constexpr Q95_32(signed char

    [[nodiscard]] constexpr friend bool operator==(Q95_32 const&, Q95_32 const&) noexcept = default;

    [[nodiscard]] constexpr friend std::strong_ordering operator<=>(Q95_32 const& lhs, Q95_32 const &rhs) noexcept
    {
        auto const tmp = lhs.lo <=> rhs.lo;
        return tmp == std::strong_ordering::equal ? lhs.hi <=> rhs.hi : tmp;
    }

    [[nodiscard]] constexpr friend Q95_32 operator+(Q95_32 const& lhs, Q95_32 const& rhs) noexcept
    {
        auto r = Q95_32{};
        auto carry = uint64_t{0};
        std::tie(r.lo(), carry) = add_carry(lhs.lo(), rhs.lo(), carry);
        std::tie(r.hi(), carry) = add_carry(lhs.hi(), rhs.hi(), carry);
        return r;
    }

    [[nodiscard]] constexpr Q95_32& operator+=(Q95_32 const& rhs) noexcept
    {
        auto carry = uint64_t{0};
        std::tie(lo(), carry) = add_carry(lo(), rhs.lo(), carry);
        std::tie(hi(), carry) = add_carry(hi(), rhs.hi(), carry);
        return *this;
    }

    [[nodiscard]] constexpr friend Q95_32 operator-(Q95_32 const& lhs, Q95_32 const& rhs) noexcept
    {
        auto r = Q95_32{};
        auto borrow = uint64_t{1};
        std::tie(r.lo(), borrow) = add_carry(lhs.lo(), ~rhs.lo(), borrow);
        std::tie(r.hi(), borrow) = add_carry(lhs.hi(), ~rhs.hi(), borrow);
        return r;
    }

    [[nodiscard]] constexpr Q95_32& operator-=(Q95_32 const& rhs) noexcept
    {
        auto borrow = uint64_t{1};
        std::tie(lo(), borrow) = add_carry(lo(), ~rhs.lo(), borrow);
        std::tie(hi(), borrow) = add_carry(hi(), ~rhs.hi(), borrow);
        return *this;
    }


};


}}
