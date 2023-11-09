// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file image/sfloat_rgba32x4.hpp Defines 4x4 matrix type sfloat_rgba32x4.
 * @ingroup image
 */

module;
#include "../macros.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>

export module hikogui_image_sfloat_rgba32x4;
import hikogui_SIMD;
import hikogui_geometry;
import hikogui_utility;

export namespace hi::inline v1 {

/** 4 x 4 x float32 pixel format.
 *
 * @ingroup image
 */
class sfloat_rgba32x4 {
public:
    constexpr sfloat_rgba32x4() = default;
    constexpr sfloat_rgba32x4(sfloat_rgba32x4 const& rhs) noexcept = default;
    constexpr sfloat_rgba32x4(sfloat_rgba32x4&& rhs) noexcept = default;
    constexpr sfloat_rgba32x4& operator=(sfloat_rgba32x4 const& rhs) noexcept = default;
    constexpr sfloat_rgba32x4& operator=(sfloat_rgba32x4&& rhs) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(sfloat_rgba32x4 const&, sfloat_rgba32x4 const&) noexcept = default;

    constexpr sfloat_rgba32x4(std::array<f32x4,4> const& rhs) noexcept
    {
        for (auto j = 0_uz; j != 4; ++j) {
            for (auto i = 0_uz; i != 4; ++i) {
                _v[j * 4 + i] = rhs[j][i];
            }
        }
    }

    constexpr sfloat_rgba32x4& operator=(std::array<f32x4, 4> const& rhs) noexcept
    {
        for (auto j = 0_uz; j != 4; ++j) {
            for (auto i = 0_uz; i != 4; ++i) {
                _v[j * 4 + i] = rhs[j][i];
            }
        }
        return *this;
    }

    constexpr sfloat_rgba32x4(matrix3 const& rhs) noexcept : sfloat_rgba32x4(static_cast<std::array<f32x4, 4>>(rhs)) {}

private:
    std::array<float, 16> _v = {};
};

} // namespace hi::inline v1
