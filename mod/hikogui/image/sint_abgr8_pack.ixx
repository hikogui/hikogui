// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file image/sint_abgr8_pack.hpp Defines the sint_abgr8_pack type.
 * @ingroup image
 */

module;
#include "../macros.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>

export module hikogui_image_sint_abgr8_pack;
import hikogui_SIMD;
import hikogui_geometry;

export namespace hi::inline v1 {

/** 4 x int8_t packed pixel format.
 *
 * @ingroup image
 */
class sint_abgr8_pack {
    uint32_t v = 0;

public:
    constexpr sint_abgr8_pack() = default;
    constexpr sint_abgr8_pack(sint_abgr8_pack const &rhs) noexcept = default;
    constexpr sint_abgr8_pack(sint_abgr8_pack &&rhs) noexcept = default;
    constexpr sint_abgr8_pack &operator=(sint_abgr8_pack const &rhs) noexcept = default;
    constexpr sint_abgr8_pack &operator=(sint_abgr8_pack &&rhs) noexcept = default;

    constexpr explicit sint_abgr8_pack(uint32_t const &rhs) noexcept : v(rhs) {}
    constexpr sint_abgr8_pack &operator=(uint32_t const &rhs) noexcept
    {
        v = rhs;
        return *this;
    }
    constexpr explicit operator uint32_t() noexcept
    {
        return v;
    }

    constexpr explicit sint_abgr8_pack(f32x4 const &rhs) noexcept : v(std::bit_cast<decltype(v)>(i8x4{rhs})) {}

    constexpr sint_abgr8_pack &operator=(f32x4 const &rhs) noexcept
    {
        v = std::bit_cast<decltype(v)>(i8x4{rhs});
        return *this;
    }

    constexpr explicit sint_abgr8_pack(corner_radii const &rhs) noexcept : sint_abgr8_pack(static_cast<f32x4>(rhs)) {}

    [[nodiscard]] constexpr friend bool operator==(sint_abgr8_pack const &lhs, sint_abgr8_pack const &rhs) noexcept = default;
};

} // namespace hi::inline v1
