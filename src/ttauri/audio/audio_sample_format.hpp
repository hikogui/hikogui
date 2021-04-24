// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt {

struct audio_sample_format {
    enum class numeric_type : uint8_t { signed_int, fixed_point, floating_point };

    /** The number of channels in an interleaved group.
     * Must be 1 or heigher.
     */
    int num_channels;

    /** The number of significant bits of the ADC/DAC.
     * It can have different meanings for different numeric formats:
     * - Signed integer: the value is aligned to the msb of the container.
     * - Unsigned integer: the value is aligned to the msb of the container.
     * - Fixed point: The number of fractional bits, the rest of the bits in the container
     *                are used as the integral bits.
     * - Floating point: Used as significant bits for dithering.
     */
    int num_bits;

    /** The number of bytes of the container.
     * Must be either 1, 2, 3 or 4
     */
    int num_bytes;

    /** The numeric type of the sample in the container.
     */
    numeric_type numeric_type;

    /** The endian order of the bytes in the container.
     */
    std::endian endian;

    /** The gain to apply before packing a sample into an integer.
     */
    [[nodiscard]] constexpr float pack_gain() const noexcept
    {
        if (numeric_type == numeric_type::floating_point) {
            return 1.0f;
        } else if (numeric_type == numeric_type::signed_int) {
            ttlet max_int = ((1_uz << (num_bits - 1)) - 1);
            ttlet max_scaled = max_int << (32 - num_bits);
            return static_cast<float>(max_scaled);
        } else if (numeric_type == numeric_type::fixed_point) {
            ttlet max_fraction = ((1_uz << num_bits) - 1);
            ttlet max_scaled = max_fraction << (32 - num_bytes * 8);
            return static_cast<float>(max_scaled);
        } else {
            tt_no_default();
        }
    }

    /** The gain to apply after unpacking an integer sample.
     */
    [[nodiscard]] constexpr float unpack_gain() const noexcept
    {
        return 1.0f / pack_gain();
    }
};

} // namespace tt
