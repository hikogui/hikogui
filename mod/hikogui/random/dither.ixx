// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <iterator>
#include <exception>

export module hikogui_random_dither;
import hikogui_SIMD;
import hikogui_random_xorshift128p;
import hikogui_utility;

export namespace hi::inline v1 {

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
        hi_not_implemented();
        return f32x4{};
        //if (to_bool(++_counter & 1)) {
        //    hilet rand = _state.next<u64x2>();
        //    hilet spdf = i16x16{bit_cast<i8x16>(rand)};
        //    hilet [spdf1, spdf2] = spdf.split<int16_t>();
        //
        //    _tpdf = spdf1 + spdf2;
        //    return f32x4{i32x4{_tpdf}} * _multiplier;
        //} else {
        //    hilet second_tpdf = bit_cast<i16x8>(bit_cast<u64x2>(_tpdf).yx());
        //    return f32x4{i32x4{second_tpdf}} * _multiplier;
        //}
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
