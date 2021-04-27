// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../architecture.hpp"
#include "../geometry/numeric_array.hpp"
#include <cstddef>
#include <bit>

namespace tt {

class audio_sample_unpacker {
public:
    /** Audio sample unpacker
     * One instance of this class can be used to unpack multiple buffers either
     * from one audio-proc to the next, or for each channel in a group of
     * interleaved channels.
     *
     * @param num_bytes
     *        Number of bytes that a sample occupies
     * @param num_integer_bits
     *        Number of bits on the left of the period for fixed point samples.
     *        This value must be 0 in signed integer format.
     * @param num_fraction_bits
     *        Number of bits on the right of the period for fixed point samples.
     *        This value is the number of bits in signed integer format.
     * @param is_float
     *        True if the value is floating point.
     *        False if the sample format is signed integer or fixed point format.
     * @param endian
     *        The byte ordering of the sample.
     * @param stride
     *        The number of bytes to step to the next sample in the same channel.
     *
     */
    audio_sample_unpacker(
        int num_bytes,
        int num_integer_bits,
        int num_fraction_bits,
        bool is_float,
        std::endian endian,
        int stride) noexcept;

    /** Unpack samples.
     *
     * @param src A pointer to a byte array containing samples.
     * @param dst A pointer to a array of floating point samples of a single channel.
     * @param num_samples Number of samples.
     */
    void operator()(std::byte const *tt_restrict src, float *tt_restrict dst, size_t num_samples) const noexcept;

private:
    int _num_bytes;
    int _num_integer_bits;
    int _num_fraction_bits;
    bool _is_float;
    std::endian _endian;
    int _stride;

    int _num_samples_per_load;
    i8x16 _shuffle_load;
    i8x16 _shuffle_shift;
    f32x4 _gain;
};

} // namespace tt
