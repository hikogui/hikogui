// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include "../cast.hpp"
#include <bit>

namespace tt {

/** Audio sample format.
 * Audio samples described by this type can be in three different formats.
 *   - Signed integer PCM, which will be treated like a fixed point
 *     where the `num_integer_bits` is set to zero. This is the format used
 *     in most audio file formats.
 *   - Fixed point PCM, a more generic format than signed integers which has
 *     some head-room/guard bits above normalized signed integers. The Q8.23 fixed integer
 *     format is used by iOS as a sample format.
 *   - Floating point PCM, the floating point format used internally in ttauri,
 *     useful for doing calculations in.
 *
 * Sample values are aligned to the most significant bits of the container described by
 * `num_bytes`. The bottom bits are set to zero.
 */
struct audio_sample_format {
    /** The number of bytes of the container.
     * Must be either 1, 2, 3 or 4
     */
    int num_bytes;

    /** The number of bits used for the integer part of a fixed point number.
     * This value is zero for signed integer and float samples.
     */
    int num_guard_bits;

    /** The number of significant bits of the sample format.
     * This value is excluding the sign.
     * `(1 << num_bits) - 1` is the maximum sample value.
     *
     * Examples:
     *  - 16 bit signed PCM -> num_bits=15
     *  - 24 bit signed PCM -> num_bits=23
     *  - float PCM -> num_bits=23
     *  - Q8.23 PCM -> num_bits=23
     */
    int num_bits;

    /** The numeric type is floating point.
     * Otherwise it is a signed integer or fixed point number.
     */
    bool is_float;

    /** The endian order of the bytes in the container.
     */
    std::endian endian;

    /** The number of bytes to step to the next sample of the same channel.
     */
    int stride;

    [[nodiscard]] constexpr float pack_multiplier() const noexcept
    {
        tt_axiom(is_valid());

        if (is_float) {
            return 1.0f;

        } else {
            // Find the maximum value of the fraction bits as a signed number.
            auto max_value = (1_uz << num_bits) - 1;

            // Align left inside an int32_t.
            max_value <<= 31 - num_bits - num_guard_bits;

            return narrow_cast<float>(max_value);
        }
    }

    [[nodiscard]] constexpr float unpack_multiplier() const noexcept
    {
        return 1.0f / pack_multiplier();
    }

    /** The number of samples that are read in a single 128 bit load.
     * Always a power of two and up to maximum 4.
     */
    [[nodiscard]] constexpr int samples_per_load() const noexcept
    {
        tt_axiom(is_valid());

        return narrow_cast<int>(std::bit_floor((((16u - num_bytes) / stride) & 3) + 1));
    }

    /** The number of bytes to advance the load for the next set of samples.
     */
    [[nodiscard]] constexpr int load_stride() const noexcept
    {
        return stride * samples_per_load();
    }

    /** The number of loads to do before we can do a 4 sample store.
     */
    [[nodiscard]] constexpr int loads_per_store() const noexcept
    {
        return 4 / samples_per_load();
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return (num_bytes >= 1 && num_bytes <= 4) && (num_bits + num_guard_bits <= num_bytes * 8) &&
            (endian == std::endian::little || endian == std::endian::big) && (stride >= num_bytes);
    }
};

} // namespace tt
