// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "xorshift128p.hpp"
#include "../required.hpp"

namespace tt {

/** An object that create dither values to add to samples before rounding.
 *
 * Dither is created by adding two 8 bit RPDF into a 9 bit TPDF. Then
 * this 9 bit TPDF is converted to floating point, which can be added to
 * the original floating point sample.
 *
 * We start of with 128 bit from an xorshift128p random number generated
 * that is split into 8 bit chunks.
 *
 */
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
        maximum_value *= 127.0f;

        // Triangular probability density function is has twice the range.
        maximum_value *= 2.0f;

        _multiplier = f32x8::broadcast(1.0f / maximum_value);
    }

    /** Get 4 floating point number to add to a samples.
     * The dither is a TPDF with the maximum being 2 quantization steps.
     */
    [[nodiscard]] f32x8 next() noexcept
    {
        auto rand = _state.next<u64x2>();
        auto spdf1 = i16x8{bit_cast<i8x16>(rand)};
        rand = rand.yx();
        auto spdf2 = i16x8{bit_cast<i8x16>(rand)};

        auto tpdf = bit_cast<u64x2>(spdf1 + spdf2);
        auto tpdf1 = i32x4{bit_cast<i16x8>(tpdf)};
        tpdf = tpdf.yx();
        auto tpdf2 = i32x4{bit_cast<i16x8>(tpdf)};

        return f32x8{i32x8{tpdf1, tpdf2}} * _multiplier;
    }

private:
    f32x8 _multiplier;
    xorshift128p _state;
};

} // namespace tt
