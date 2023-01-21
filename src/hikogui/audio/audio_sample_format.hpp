// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../SIMD/module.hpp"
#include <bit>

namespace hi::inline v1 {

/** Audio sample format.
 * Audio samples described by this type can be in three different formats.
 *   - Signed integer PCM, which will be treated like a fixed point
 *     where the `num_integer_bits` is set to zero. This is the format used
 *     in most audio file formats.
 *   - Fixed point PCM, a more generic format than signed integers which has
 *     some head-room/guard bits above normalized signed integers. The Q8.23 fixed integer
 *     format is used by iOS as a sample format.
 *   - Floating point PCM, the floating point format used internally in hikogui,
 *     useful for doing calculations in.
 *
 * Sample values are aligned to the most significant bits of the container described by
 * `num_bytes`. The bottom bits are set to zero.
 */
struct audio_sample_format {
    /** The number of bytes of the container.
     * Must be either 1, 2, 3 or 4
     */
    uint8_t num_bytes;

    /** The number of bits used for the integer part of a fixed point number.
     * This value is zero for signed integer and float samples.
     */
    uint8_t num_guard_bits;

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
    uint8_t num_bits;

    /** The numeric type is floating point.
     * Otherwise it is a signed integer or fixed point number.
     */
    bool is_float;

    /** The endian order of the bytes in the container.
     */
    std::endian endian;

    constexpr audio_sample_format() noexcept :
        num_bytes(0), num_guard_bits(0), num_bits(0), is_float(false), endian(std::endian::native)
    {
    }

    constexpr audio_sample_format(audio_sample_format const &) noexcept = default;
    constexpr audio_sample_format(audio_sample_format &&) noexcept = default;
    constexpr audio_sample_format &operator=(audio_sample_format const &) noexcept = default;
    constexpr audio_sample_format &operator=(audio_sample_format &&) noexcept = default;

    /** Constructor of an audio sample format.
     *
     * @param num_bytes The number of bytes used for each sample in the stream of data.
     * @param num_guard_bits The number of bits used beyond -1.0 and 1.0.
     * @param num_bits The number of bits used to represent a normalized sample between 0.0 and 1.0 (without the sign bit).
     * @param is_float True if the sample is float, otherwise integer or fixed point.
     * @param endian The ordering of bytes in each sample
     */
    [[nodiscard]] constexpr audio_sample_format(
        uint8_t num_bytes,
        uint8_t num_guard_bits,
        uint8_t num_bits,
        bool is_float,
        std::endian endian) noexcept :
        num_bytes(num_bytes), num_guard_bits(num_guard_bits), num_bits(num_bits), is_float(is_float), endian(endian)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] static constexpr audio_sample_format float32_le() noexcept
    {
        return {4, 8, 23, true, std::endian::little};
    }

    [[nodiscard]] static constexpr audio_sample_format float32_be() noexcept
    {
        return {4, 8, 23, true, std::endian::big};
    }

    [[nodiscard]] static constexpr audio_sample_format float32() noexcept
    {
        return {4, 8, 23, true, std::endian::native};
    }

    [[nodiscard]] static constexpr audio_sample_format int16_le() noexcept
    {
        return {2, 0, 15, false, std::endian::little};
    }

    [[nodiscard]] static constexpr audio_sample_format int16_be() noexcept
    {
        return {2, 0, 15, false, std::endian::big};
    }

    [[nodiscard]] static constexpr audio_sample_format int16() noexcept
    {
        return {2, 0, 15, false, std::endian::native};
    }

    [[nodiscard]] static constexpr audio_sample_format int20_le() noexcept
    {
        return {3, 0, 19, false, std::endian::little};
    }

    [[nodiscard]] static constexpr audio_sample_format int20_be() noexcept
    {
        return {3, 0, 19, false, std::endian::big};
    }

    [[nodiscard]] static constexpr audio_sample_format int20() noexcept
    {
        return {3, 0, 19, false, std::endian::native};
    }

    [[nodiscard]] static constexpr audio_sample_format int24_le() noexcept
    {
        return {3, 0, 23, false, std::endian::little};
    }

    [[nodiscard]] static constexpr audio_sample_format int24_be() noexcept
    {
        return {3, 0, 23, false, std::endian::big};
    }

    [[nodiscard]] static constexpr audio_sample_format int24() noexcept
    {
        return {3, 0, 23, false, std::endian::native};
    }

    [[nodiscard]] static constexpr audio_sample_format int32_le() noexcept
    {
        return {4, 0, 31, false, std::endian::little};
    }

    [[nodiscard]] static constexpr audio_sample_format int32_be() noexcept
    {
        return {4, 0, 31, false, std::endian::big};
    }

    [[nodiscard]] static constexpr audio_sample_format int32() noexcept
    {
        return {4, 0, 31, false, std::endian::native};
    }

    [[nodiscard]] static constexpr audio_sample_format fix8_23_le() noexcept
    {
        return {4, 8, 23, false, std::endian::little};
    }

    [[nodiscard]] static constexpr audio_sample_format fix8_23_be() noexcept
    {
        return {4, 8, 23, false, std::endian::big};
    }

    [[nodiscard]] static constexpr audio_sample_format fix8_23() noexcept
    {
        return {4, 8, 23, false, std::endian::native};
    }

    constexpr explicit operator bool() const noexcept
    {
        return num_bytes != 0;
    }

    /** How much to multiply float samples to create integer samples.
     */
    [[nodiscard]] float pack_multiplier() const noexcept;

    /** How much to multiply integer samples to create float samples.
     */
    [[nodiscard]] float unpack_multiplier() const noexcept;

    /** The number of packed samples that are handled in a single 128 bit load or store.
     * Always one of: 1, 2 or 4.
     */
    [[nodiscard]] std::size_t num_samples_per_chunk(std::size_t stride) const noexcept;

    /** The number of bytes to advance to the next chunk to be loaded or stored.
     */
    [[nodiscard]] std::size_t chunk_stride(std::size_t stride) const noexcept;

    /** The number of chunks to load or store to handle 4 samples.
     */
    [[nodiscard]] std::size_t num_chunks_per_quad(std::size_t stride) const noexcept;

    /** Calculate the number of 4 sample-quads can be handled as chunked loads and stores.
     */
    [[nodiscard]] std::size_t num_fast_quads(std::size_t stride, std::size_t num_samples) const noexcept;

    /** Return a shuffle indices for loading samples into 32 bit integers.
     */
    [[nodiscard]] i8x16 load_shuffle_indices(std::size_t stride) const noexcept;

    /** Return a shuffle indices for storing 32 bit samples into packed samples.
     */
    [[nodiscard]] i8x16 store_shuffle_indices(std::size_t stride) const noexcept;

    /** Return a shuffle indices to shift previous loaded samples for concatenation.
     */
    [[nodiscard]] i8x16 concat_shuffle_indices(std::size_t stride) const noexcept;

    /** Is the audio sample format valid.
     */
    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return (num_bytes >= 1 && num_bytes <= 4) && (num_bits + num_guard_bits <= num_bytes * 8) &&
            (endian == std::endian::little || endian == std::endian::big);
    }
};

} // namespace hi::inline v1
