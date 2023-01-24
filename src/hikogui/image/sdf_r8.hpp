// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file image/sdf_r8.hpp Defines the signed distance field pixel type sdf_r8.
 * @ingroup image
 */

#pragma once

#include "snorm_r8.hpp"
#include "../utility/module.hpp"

hi_warning_push();
// C26434: Function '...' hides a non-virtual function '...'.
// We need to hide those functions.
hi_warning_ignore_msvc(26434);

namespace hi::inline v1 {

/** A pixel of a single channel signed distance field.
 * https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
 *
 * @ingroup image
 */
struct sdf_r8 : public snorm_r8 {
    /** Max distance in pixels represented by the signed distance field.
     * The max_distance determines the maximum size of the shadow compared to the
     * font as drawn into the atlas. The maximum distance needs to be at least
     * the diagonal distance between two pixels for proper linear-interpolation sqrt(1.0*1.0 + 1.0*1.0).
     */
    constexpr static float max_distance = 3.0f;
    constexpr static float one_over_max_distance = 1.0f / max_distance;

    sdf_r8() noexcept = default;
    sdf_r8(sdf_r8 const& other) noexcept = default;
    sdf_r8(sdf_r8&& other) noexcept = default;
    sdf_r8& operator=(sdf_r8 const& other) noexcept = default;
    sdf_r8& operator=(sdf_r8&& other) noexcept = default;
    ~sdf_r8() = default;

    sdf_r8(float rhs) noexcept : snorm_r8(rhs * one_over_max_distance) {}

    sdf_r8& operator=(float rhs) noexcept
    {
        snorm_r8::operator=(rhs *one_over_max_distance);
        return *this;
    }

    operator float() const noexcept
    {
        return (snorm_r8::operator float()) * max_distance;
    }

    void repair() noexcept
    {
        *this = -static_cast<float>(*this);
    }
};

} // namespace hi::inline v1

hi_warning_pop();
