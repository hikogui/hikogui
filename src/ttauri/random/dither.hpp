// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "xorshift128p.hpp"
#include "../required.hpp"

namespace tt {

class dither {
public:
    dither(dither const &) = default;
    dither(dither &&) = default;
    dither &operator=(dither const &) = default;
    dither &operator=(dither &&) = default;

    /** Create a dither object.
     *
     * @param num_bits Number of significant fraction bits, excluding the sign bit.
     *                 For 24 bit signed PCM samples this value is 23.
     */
    dither(int num_bits) noexcept : _state(), _multiplier()
    {
        tt_axiom(num_bits > 0);
        auto maximum_value = static_cast<float>((1_uz << num_bits) - 1);

        // The maximum value from the rectangular probability density function.
        // We shifted right by 1 to not overflow the triangle probably density function.
        maximum_value *= 16383.0f;

        // Triangular probability density function is has twice the range.
        maximum_value *= 2.0f;

        _multiplier = f32x4::broadcast(1.0f / maximum_value);
    }

    /** Get 4 floating point number to add to a samples.
     * The dither is a TPDF with the maximum being 2 quantization steps.
     */
    [[nodiscard]] f32x4 next() noexcept
    {
        auto rpdf_i16x8 = _state.next<i16x8>() >> 1;
        auto tpdf_i16x4 = hadd(rpdf_i16x8, rpdf_i16x8);
        auto tpdf_i32x4 = static_cast<i32x4>(tpdf_i16x4);
        return static_cast<f32x4>(tpdf_i32x4) * _multiplier;
    }

private:
    xorshift128p _state;
    f32x4 _multiplier;
};

} // namespace tt
