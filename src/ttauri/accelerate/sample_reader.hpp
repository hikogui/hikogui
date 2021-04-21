// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>
#include <cstdint>
#include <bit>
#include <memory>
#include <numeric>
#include "../geometry/numeric_array.hpp"

namespace tt {

/*
 * To read and process samples as quick as possible we should read
 * them in chunks of 4x4 sample/channels.
 *
 * This allows writing of 4 samples in a go, while also being able to
 * do filter with 4 channels in parallel.
 */


[[nodiscard]] constexpr float float_to_int_multiplier(size_t nr_bits) noexcept
{
    tt_axiom(nr_bits >= 9, "sample formats of 8 bit and smaller are unsigned");
    tt_axiom(nr_bits <= (sizeof(size_t) * CHAR_BIT));

    return static_cast<float>((1_uz << (nr_bits - 1)) - 1);
}

[[nodiscard]] constexpr float int_to_float_multiplier(size_t nr_bits) noexcept
{
    return 1.0f / float_to_int_multiplier(nr_bits);
}



/** A class to read samples from memory.
 * @tparam NumBytes Number of bytes of the container of the sample.
 * @tparam NumBits Number of significant bits of the sample.
 * @tparam Normalized True if the sample is aligned to the msb of the container.
 * @tparam FloatingPoint the samples are in floating point.
 * @tparam Endian The endian ordering of the container
 * @tparam Stride Number of bytes to advance to the next container.
 */
template<size_t NumBytes, size_t NumBits, bool Normalized, bool FloatingPoint, std::endian Endian, size_t Stride>
class sample_reader {
private:
    /** Number of bytes that are unused in a int32_t container.
     */
    constexpr static size_t _num_int32_unused_bytes = sizeof(int32_t) - NumBytes;

    /** Number of bits that are unused in a int32_t container.
     */
    constexpr static size_t _num_int32_unused_bits = sizeof(int32_t) * 8 - NumBits;

    /** The number of unused msb-bits after placing the sample into a int32_t container.
     */
    constexpr static size_t _num_int32_unused_msb_bits = Normalized ? _num_int32_unused_bytes * 8 : _num_int32_unused_bits;

    /** The maximum value of the sample.
     */
    constexpr static size_t _max_sample_value = (1_uz << (NumBits - 1)) - 1;

    /** The maximum value of the sample after normalizing to a int32_t.
     */
    constexpr static size_t _max_int32_sample_value = _max_sample_value << _num_int32_unused_bits;

    /** The multiplier to use to convert a shifted sample value to a normalized float between -1.0 and 1.0.
     */
    constexpr static float _multiplier = FloatingPoint ? 1.0f : 1.0f / static_cast<float>(_max_int32_sample_value);

    /** The number of integers which are read in a single 128 bit load.
     */
    constexpr static size_t _num_samples_per_lane = 4 * Stride + NumBytes <= 16 ? 4 : 2 * Stride + NumBytes <= 16 ? 2 : 1;

    /** Number of samples in a i32x4 vector that are unused after a read.
     */
    constexpr static size_t _num_unused_samples_per_lane = 4 - _num_samples_per_lane;

    /** The stride for each 128 bit load.
     */
    constexpr static size_t _stride_per_lane = _num_samples_per_lane * Stride;

public:
    /** Default constructor.
     * The default constructor will initialize the vector multiplier and permute mask.
     * Just to make sure that the two member variables are assigned to a CPU register
     * before the inner-loop starts, this constructor is purposefully NOT constexpr.
     */
    sample_reader() noexcept = default;

    template<typename R>
    [[nodiscard]] constexpr R read(std::byte const *&ptr) const noexcept
    {
        tt_not_implemented();
    }

    /** Read 1 sample from memory.
     * @param [in,out]ptr The pointer to the sample, afterwards the ptr is advanced to the next sample.
     * @return The sample read from memory.
     */
    template<>
    [[nodiscard]] constexpr int32_t read(std::byte const *&ptr) const noexcept
    {
        int32_t r;

        if (Endian == std::endian::little) {
            uint32_t tmp = 0;
            for (size_t i = 0; i != NumBytes; ++i) {
                tmp |= static_cast<uint32_t>(*(ptr + i)) << (i * 8);
            }

            r = static_cast<int32_t>(tmp);

        } else {
            uint32_t tmp = 0;
            for (size_t i = 0; i != NumBytes; ++i) {
                tmp <<= 8;
                tmp |= static_cast<uint32_t>(*(ptr + i));
            }

            r = static_cast<int32_t>(tmp);
        }

        ptr += Stride;

        // Msb-align the sample, this will make the sign bit valid.
        if constexpr (_num_int32_unused_msb_bits != 0) {
            r <<= _num_int32_unused_msb_bits;
        }
        return r;
    }

    /** Read 1 signed sample from memory as float.
     * @param [in,out]ptr The pointer to the sample, afterwards the ptr is advanced to the next sample.
     * @return The sample read from memory.
     */
    template<>
    [[nodiscard]] constexpr float read(std::byte const *&ptr) const noexcept
    {
        if constexpr (FloatingPoint) {
            return std::bit_cast<float>(read<int32_t>(ptr));
        } else {
            return static_cast<float>(read<int32_t>(ptr)) * _multiplier;
        }
    }

    /** Read 4 signed samples from memory.
     * @param [in,out]ptr The pointer to the first of four samples, afterwards the ptr is advanced to the next sample.
     * @return 4 samples.
     */
    template<>
    [[nodiscard]] constexpr i32x4 read(std::byte const *&ptr) const noexcept
    {
        tt_axiom(is_valid());
        tt_axiom(ptr != nullptr);

        auto r = i32x4{};

        for (size_t i = 0; i != 4; i += _num_samples_per_lane, ptr += _stride_per_lane) {
            i8x16 bytes;
            std::memcpy(&bytes, ptr, sizeof(bytes));

            if constexpr (Endian != std::endian::native || NumBytes != 4 || Stride != 4) {
                // The permute mask will make the samples 32 bit integers, including endian change.
                // Unused samples are set to zero and are placed in the low
                // indices of the vector, this way we can swizzle tmp in the low direction
                // before or-ing the bytes on top to get the correct ordering.
                bytes = shuffle(bytes, _permute_mask);
            }

            // Shift the previous iteration to lower indices.
            if constexpr (_num_samples_per_lane == 1) {
                r = r.yzw0();
            } else if constexpr (_num_samples_per_lane == 2) {
                r = r.zw00();
            }

            r |= std::bit_cast<i32x4>(bytes);
        }

        if constexpr (_num_int32_unused_msb_bits != 0) {
            r <<= _num_int32_unused_msb_bits;
        }
        return r;
    }

    /** Read 8 samples from memory.
     * @param [in,out]ptr The pointer to the first of four samples, afterwards the ptr is advanced to the next sample.
     * @return 8 samples.
     */
    template<>
    [[nodiscard]] constexpr i32x8 read(std::byte const *&ptr) const noexcept
    {
        ttlet r0 = read<i32x4>(ptr);
        ttlet r1 = read<i32x4>(ptr);
        return {r0, r1};
    }

    /** Read 8 samples from memory as floats.
     * @param [in,out]ptr The pointer to the first of four samples, afterwards the ptr is advanced to the next sample.
     * @return 8 samples.
     */
    template<>
    [[nodiscard]] constexpr f32x8 read(std::byte const *&ptr) const noexcept
    {
        if constexpr (FloatingPoint) {
            return std::bit_cast<f32x8>(read<i32x8>(ptr));
        } else {
            return static_cast<f32x8>(read<i32x8>(ptr)) * _multiplier_f32x8;
        }
    }

private:
    /** Multiplier to use to convert samples into normalized floating point numbers between -1.0 and 1.0.
     */
    f32x8 _multiplier_f32x8 = f32x8::broadcast(_multiplier);

    /** Permute mask used to extract and endian-swap samples into i32x4 vector.
     */
    i8x16 _permute_mask = make_permute_mask();

    [[nodiscard]] static constexpr i8x16 make_permute_mask()
    {
        i8x16 r;

        for (size_t dst_i = 0; dst_i != 16; ++dst_i) {
            // clang-format off

            // The unused parts of the vector should be in the low parts, so that we can do a
            // simple shift-then-or trick to assemble the full vector.
            ttlet sample_index = static_cast<ssize_t>(dst_i / 4) - _num_unused_samples_per_lane;

            // The byte index inside the sample counting from the least-significant byte.
            ttlet sample_byte_index = (std::endian::native == std::endian::little) ?
                dst_i % 4 :
                3 - (dst_i % 4);

            if (sample_index >= 0 && sample_byte_index < NumBytes) {
                ttlet src_i = (Endian == std::endian::little) ? 
                    sample_index * Stride + sample_byte_index :
                    sample_index * Stride + (NumBytes - 1 - sample_byte_index);

                permute_mask[dst_i] = narrow_cast<int8_t>(src_i);

            } else {
                // Any other byte is set to zero.
                permute_mask[dst_i] = -1;
            }
            // clang-format on
        }

        return r;
    }
};

} // namespace tt
