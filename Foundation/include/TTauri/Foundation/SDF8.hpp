// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/R8SNorm.hpp"
#include "TTauri/Foundation/math.hpp"

namespace TTauri {

/** A pixel of a single channel signed distance field.
 * https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
 */
struct SDF8 : public R8SNorm {
    /** Max distance in pixels represented by the signed distance field.
     * The max_distance determines the maximum size of the shadow compared to the
     * font as drawn into the atlas. The maximum distance needs to be at least
     * the diagonal distance between two pixels for proper linear-interpolation sqrt(1.0*1.0 + 1.0*1.0).
     */
    constexpr static float max_distance = 2.0f;
    constexpr static float one_over_max_distance = 1.0f / max_distance;

    SDF8() noexcept = default;
    SDF8(SDF8 const &other) noexcept = default;
    SDF8(SDF8 &&other) noexcept = default;
    SDF8 &operator=(SDF8 const &other) noexcept = default;
    SDF8 &operator=(SDF8 &&other) noexcept = default;
    ~SDF8() = default;

    SDF8(float rhs) noexcept :
        R8SNorm(rhs * one_over_max_distance) {}

    SDF8 &operator=(float rhs) noexcept {
        R8SNorm::operator=(rhs * one_over_max_distance);
        return *this;
    }

    operator float () const noexcept {
        return (R8SNorm::operator float()) * max_distance;
    }

    void repair() noexcept {
        *this = -static_cast<float>(*this);
    }
};

}