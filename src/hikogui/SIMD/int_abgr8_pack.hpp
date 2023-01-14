// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/corner_radii.hpp"
#include <algorithm>

namespace hi::inline v1 {

class int_abgr8_pack {
    uint32_t v = 0;

public:
    constexpr int_abgr8_pack() = default;
    constexpr int_abgr8_pack(int_abgr8_pack const &rhs) noexcept = default;
    constexpr int_abgr8_pack(int_abgr8_pack &&rhs) noexcept = default;
    constexpr int_abgr8_pack &operator=(int_abgr8_pack const &rhs) noexcept = default;
    constexpr int_abgr8_pack &operator=(int_abgr8_pack &&rhs) noexcept = default;

    constexpr explicit int_abgr8_pack(uint32_t const &rhs) noexcept : v(rhs) {}
    constexpr int_abgr8_pack &operator=(uint32_t const &rhs) noexcept
    {
        v = rhs;
        return *this;
    }
    constexpr explicit operator uint32_t() noexcept
    {
        return v;
    }

    constexpr explicit int_abgr8_pack(f32x4 const &rhs) noexcept : v(std::bit_cast<decltype(v)>(i8x4{rhs})) {}

    constexpr int_abgr8_pack &operator=(f32x4 const &rhs) noexcept
    {
        v = std::bit_cast<decltype(v)>(i8x4{rhs});
        return *this;
    }

    constexpr explicit int_abgr8_pack(corner_radii const &rhs) noexcept : int_abgr8_pack(static_cast<f32x4>(rhs)) {}

    [[nodiscard]] constexpr friend bool operator==(int_abgr8_pack const &lhs, int_abgr8_pack const &rhs) noexcept = default;
};

} // namespace hi::inline v1
