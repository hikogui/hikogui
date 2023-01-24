// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file image/uint_abgr8_pack.hpp Defines the uint_abgr8_pack type.
 * @ingroup image
 */

#pragma once

#include "../geometry/module.hpp"
#include <algorithm>

namespace hi::inline v1 {

/** 4 x uint8_t pixel packed format.
 *
 * @ingroup image
 */
class uint_abgr8_pack {
    uint32_t v = {};

public:
    constexpr uint_abgr8_pack() = default;
    constexpr uint_abgr8_pack(uint_abgr8_pack const &rhs) noexcept = default;
    constexpr uint_abgr8_pack(uint_abgr8_pack &&rhs) noexcept = default;
    constexpr uint_abgr8_pack &operator=(uint_abgr8_pack const &rhs) noexcept = default;
    constexpr uint_abgr8_pack &operator=(uint_abgr8_pack &&rhs) noexcept = default;

    constexpr explicit uint_abgr8_pack(uint32_t const &rhs) noexcept : v(rhs) {}
    constexpr uint_abgr8_pack &operator=(uint32_t const &rhs) noexcept
    {
        v = rhs;
        return *this;
    }
    constexpr explicit operator uint32_t() noexcept
    {
        return v;
    }

    constexpr explicit uint_abgr8_pack(f32x4 const &rhs) noexcept : v(std::bit_cast<decltype(v)>(u8x4{rhs})) {}

    constexpr uint_abgr8_pack &operator=(f32x4 const &rhs) noexcept
    {
        v = std::bit_cast<decltype(v)>(u8x4{rhs});
        return *this;
    }

    constexpr explicit uint_abgr8_pack(corner_radii const &rhs) noexcept : uint_abgr8_pack(static_cast<f32x4>(rhs)) {}

    [[nodiscard]] constexpr friend bool operator==(uint_abgr8_pack const &lhs, uint_abgr8_pack const &rhs) noexcept = default;
};

} // namespace hi::inline v1
