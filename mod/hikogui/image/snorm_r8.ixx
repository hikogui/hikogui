// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file image/snorm_r8.hpp Defines the snorm_r8.
 * @ingroup image
 */

module;
#include "../macros.hpp"

#include <algorithm>
#include <cmath>
#include <bit>
#include <cstdint>

export module hikogui_image_snorm_r8;
import hikogui_utility;

export namespace hi::inline v1 {

[[nodiscard]] constexpr int8_t make_snorm_r8_value(float rhs) noexcept
{
    return round_cast<int8_t>(std::round(std::clamp(rhs, -1.0f, 1.0f) * 127.0f));
}

/** 1 x int8_t pixel format.
 *
 * @ingroup image
 */
struct snorm_r8 {
    int8_t value;

    snorm_r8() = default;
    snorm_r8(snorm_r8 const &rhs) noexcept = default;
    snorm_r8(snorm_r8 &&rhs) noexcept = default;
    snorm_r8 &operator=(snorm_r8 const &rhs) noexcept = default;
    snorm_r8 &operator=(snorm_r8 &&rhs) noexcept = default;
    ~snorm_r8() = default;

    explicit snorm_r8(float rhs) noexcept : value(make_snorm_r8_value(rhs)) {}

    snorm_r8 &operator=(float rhs) noexcept
    {
        value = make_snorm_r8_value(rhs);
        return *this;
    }

    explicit operator float() const noexcept
    {
        return narrow_cast<float>(value) / 127.0f;
    }
};

} // namespace hi::inline v1
