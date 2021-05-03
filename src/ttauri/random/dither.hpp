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
        maximum_value *= 2147483647.0f;

        // Triangular probability density function is has twice the range.
        maximum_value *= 2.0f;

        _multiplier = f32x4::broadcast(1.0f / maximum_value);
    }

    /** Get 4 floating point number to add to a samples.
     * The dither is a TPDF with the maximum being 2 quantization steps.
     */
    [[nodiscard]] f32x4 next() noexcept
    {
        auto rpdf1 = f32x4{_state.next<i32x4>()};
        auto rpdf2 = f32x4{_state.next<i32x4>()};
        auto tpdf = rpdf1 + rpdf2;
        return tpdf * _multiplier;
    }

private:
    xorshift128p _state;
    f32x4 _multiplier;
};

} // namespace tt
