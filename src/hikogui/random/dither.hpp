// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "xorshift128p.hpp"
#include "../utility.hpp"

namespace hi::inline v1 {

/** An object that create dither values to add to samples before rounding.
 *
 * Dither is created by adding two 8 bit RPDF into a 9 bit TPDF. Then
 * this 9 bit TPDF is converted to floating point, which can be added to
 * the original floating point sample.
 *
 * We start of with 128 bit from an xorshift128p random number generated
 * that is split into 8 bit chunks, made into TPDF and converted to 8
 * floating point values.
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
    dither(int num_bits) noexcept
    {
        hi_axiom(num_bits > 0);
        auto maximum_value = static_cast<float>((1_uz << num_bits) - 1);

        // The maximum value from the rectangular probability density function.
        maximum_value *= 127.0f;

        // Triangular probability density function is has twice the range.
        maximum_value *= 2.0f;

        _multiplier = f32x4::broadcast(1.0f / maximum_value);
    }

    /** Get 8 floating point number to add to a samples.
     * The dither is a TPDF with the maximum being 2 quantization steps.
     */
    f32x4 next() noexcept
    {
        if (to_bool(++_counter & 1)) {
            auto rand = _state.next<u64x2>();
            hilet spdf1 = i16x8{bit_cast<i8x16>(rand)};
            rand = rand.yx();
            hilet spdf2 = i16x8{bit_cast<i8x16>(rand)};

            _tpdf = spdf1 + spdf2;
            return f32x4{i32x4{_tpdf}} * _multiplier;
        } else {
            auto second_tpdf = bit_cast<i16x8>(bit_cast<u64x2>(_tpdf).yx());
            return f32x4{i32x4{second_tpdf}} * _multiplier;
        }
    }

    /** Add dither to the given samples.
     *
     * @param samples The samples to add dithering to.
     * @return The sample with included dither.
     */
    [[nodiscard]] f32x4 next(f32x4 samples) noexcept
    {
        return samples + next();
    }

    /** Add dither to the given samples.
     *
     * @param sample The sample to add dithering to.
     * @return The sample with included dither.
     */
    [[nodiscard]] float next(float sample) noexcept
    {
        return get<0>(f32x4::broadcast(sample) + next());
    }

private:
    f32x4 _multiplier = {};
    i16x8 _tpdf = {};
    xorshift128p _state = {};
    unsigned char _counter = 0;
};

} // namespace hi::inline v1
