// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include "../cast.hpp"
#include "../rapid/numeric_array.hpp"
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

    /** How much to multiply float samples to create integer samples.
     */
    [[nodiscard]] float pack_multiplier() const noexcept;

    /** How much to multiply integer samples to create float samples.
     */
    [[nodiscard]] float unpack_multiplier() const noexcept;

    /** The number of packed samples that are handled in a single 128 bit load or store.
     * Always one of: 1, 2 or 4.
     */
    [[nodiscard]] int num_samples_per_chunk() const noexcept;

    /** The number of bytes to advance to the next chunk to be loaded or stored.
     */
    [[nodiscard]] int chunk_stride() const noexcept;

    /** The number of chunks to load or store to handle 4 samples.
     */
    [[nodiscard]] int num_chunks_per_quad() const noexcept;

    /** Calculate the number of 4 sample-quads can be handled as chunked loads and stores.
     */
    [[nodiscard]] size_t num_fast_quads(size_t num_samples) const noexcept;


    /** Return a shuffle indices for loading samples into 32 bit integers.
     */
    [[nodiscard]] i8x16 unpack_load_shuffle_indices() const noexcept;

    /** Return a shuffle indices for storing 32 bit samples into packed samples.
     */
    [[nodiscard]] i8x16 pack_store_shuffle_indices() const noexcept;

    /** Return a shuffle indices to shift previous loaded samples for concatenation.
     */
    [[nodiscard]] i8x16 unpack_concat_shuffle_indices() const noexcept;

    /** Return a shuffle indices to shift previous loaded samples for concatenation.
     */
    [[nodiscard]] i8x16 pack_split_shuffle_indices() const noexcept;

    /** Is the audio sample format valid.
     */
    [[nodiscard]] bool is_valid() const noexcept;
};

} // namespace tt
