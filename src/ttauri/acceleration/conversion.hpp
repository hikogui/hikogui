// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <bit>
#include <memory>
#include "../geometry/numeric_array.hpp"

namespace tt {

inline float hadd(__m256 x)
{
    return _mm256_cvtss_f32(_mm256_hadd_ps(sum_v));
}

inline float hmax(__m256 x)
{
    auto peak_l = _mm256_extractf128_ps(peak_v, 0);
    auto peak_r = _mm256_extractf128_ps(peak_v, 1);
    peak_l = _mm_max_ps(peak_l, peak_r);
    peak_r = _mm_permute_ps(peak_l, 0b00'00'11'10);
    peak_l = _mm_max_ps(peak_l, peak_r);
    peak_r = _mm_permute_ps(peak_l, 0b00'00'00'01);
    peak_l = _mm_max_ps(peak_l, peak_r);
    float peak_s = _mm_cvtss_f32(peak_l, 0);
}

struct convert_result {
    float peak;
    float mean;
};

struct load_samples_context {
    /** Multiplier to use to convert integers into normalized floating point numbers between -1.0 and 1.0.
     */
    f32x8 multiplier;

    /** Permute mask used to extract, extend & endian-swap short integers into int32_t.
     */
    i8x16 permute_mask;

    /** How much to advance the input pointer after each 128-bit load.
     */
    size_t load_stride;

    /** How many samples are loaded with each 128-bit load.
     */
    size_t samples_per_load;

    [[nodiscard]] static constexpr i8x16
    make_permute_mask(size_t num_bytes_per_sample, std::endian endian, size_t samples_per_load, size_t stride)
    {
        i8x16 r;

        for (size_t dst_i = 0; dst_i != 16; ++dst_i) {
            // clang-format off
            ttlet sample_index = dst_i / 4;

            // The byte index inside the sample counting from the least-significant byte.
            ttlet sample_byte_index = (std::endian::native == std::endian::little) ?
                dst_i % 4 :
                3 - (dst_i % 4);

            if (sample_index < samples_per_load && sample_byte_index < num_bytes_per_sample) {
                ttlet src_i = (endian == std::endian::little) ? 
                    sample_index * stride + sample_byte_index :
                    sample_index * stride + (num_bytes_per_sample - 1 - sample_byte_index);

                permute_mask[dst_i] = narrow_cast<int8_t>(src_i);

            } else {
                // Any other byte is set to zero.
                permute_mask[dst_i] = -1;
            }
            // clang-format on
        }

        return r;
    }

    [[nodiscard]] static constexpr calculate_samples_per_load(size_t num_bytes_per_sample, size_t stride) noexcept
    {
        // Determine the amount of samples that can be loaded in a single 128-bit load.
        if ((4 * stride + num_bytes_per_sample) <= 16) {
            return 4;
        } else if ((2 * stride + num_bytes_per_sample) <= 16) {
            return 2;
        } else {
            return 1;
        }
    }

    /** Initialize a context for loading signed integer samples.
     * @param num_bytes_per_sample The number of byte that is used to represent an audio sample.
     * @param num_bit_per_sample The number of significant bits of the audio sample.
     * @param sample_is_normalized If true the significant bits of the audio sample is scaled up
     *        to fit inside the number of bytes available.
     * @param endian The byte order of the sample in memory.
     * @param stride The number of bytes to increment a pointer to the next sample.
     */
    [[nodiscard]] static constexpr load_samples_context int_samples(
        size_t num_bytes_per_sample,
        size_t num_bits_per_sample,
        bool sample_is_normalized,
        std::endian endian,
        size_t stride) noexcept
    {
        // The multiplier is based on the actual number of bits in the sample
        auto max_sample_value = (1_uz << (num_bits_per_sample - 1)) - 1;
        if (sample_is_normalized) {
            max_sample_value <<= (num_bytes_per_sample * 8 - num_bits);
        }

        multiplier = f32x8::broadcast(1.0f / static_cast<float>(max_sample_value));

        samples_per_load = calculate_samples_per_load(num_bytes_per_samples, stride);

        load_stride = stride * samples_per_load;

        permute_mask = make_permute_mask(num_bytes_per_sample, endian, samples_per_load, stride);
    }

    [[nodiscard]] constexpr load_samples_context float_samples(size_t stride) noexcept
    {
        multiplier = f32x8::broadcast(1.0f);

        samples_per_load = calculate_samples_per_load(sizeof(float), stride);

        load_stride = stride * samples_per_load;

        permute_mask = make_permute_mask(sizeof(float), std::endian::native, samples_per_load, stride);
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return load_stride != 0 && (samples_per_load == 1 || samples_per_load == 2 || samples_per_load == 4);
    }
};

/** Load 4 signed integer samples from memory.
 * @param [in,out]ptr The pointer to the first sample of four samples.
 * @param context The context to use to extract the sample from memory.
 * @return 4 signed integer samples.
 */
[[nodiscard]] constexpr i32x4 load_4_samples(int8_t const *&ptr, load_samples_context const &context) noexcept
{
    tt_axiom(context.is_valid());
    tt_axiom(ptr != nullptr);

    auto r = i8x16{};

    for (size_t i = 0; i != 4; i += context.samples_per_load) {
        ttlet packed_samples = i8x16(ptr);
        ptr += context.load_stride;

        // The permute mask will make 32 bit integers, including endian change.
        // Unused integers are set to zero.
        ttlet i32_samples = shuffle(packed_samples, context.permute_mask);

        // Insert the integers into r0
        r |= byte_shift_left(i32_samples, i * 4);
    }

    return std::bit_cast<i32x4>(r);
}

/** Load 8 signed integer samples from memory.
 * @param [in,out]ptr The pointer to the first sample of eight samples.
 * @param context The context to use to extract the sample from memory.
 * @return 8 signed integer samples.
 */
[[nodiscard]] constexpr i32x8 load_8_samples(int8_t const *&ptr, load_samples_context const &context) noexcept
{
    ttlet r0 = load_4_samples(ptr, context);
    ttlet r1 = load_4_samples(ptr, context);
    return {r0, r1};
}

/** Load 8 signed integer samples from memory.
 * @param [in,out]ptr The pointer to the first sample of eight samples.
 * @param context The context to use to extract the sample from memory.
 * @return 8 normalized floating point samples.
 */
[[nodiscard]] constexpr f32x8 load_8_int_samples(int8_t const *&ptr, load_samples_context const &context) noexcept
{
    ttlet int_samples = f32x8{load_8_samples(int8_t const *&ptr, load_samples_context const &context)};
    return f32x8{int_samples} * context.multiplier;
}

/** Load 8 floating-point samples from memory.
 * @param [in,out]ptr The pointer to the first sample of eight samples.
 * @param context The context to use to extract the sample from memory.
 * @return 8 normalized floating point samples.
 */
[[nodiscard]] constexpr f32x8 load_8_float_samples(int8_t const *&ptr, load_float_samples_context const &context) noexcept
{
    return std::bit_cast<f32x8>(load_8_samples(int8_t const *&ptr, load_samples_context const &context));
}

/** Load 8 floating-point samples from memory.
 * @param [in,out]ptr The pointer to the first sample of eight samples.
 * @return 8 normalized floating point samples.
 */
[[nodiscard]] constexpr f32x8 load_8_float_samples(int8_t const *&ptr) noexcept
{
    ttlet r = f32x8{reinterpret_cast<float const *>(ptr)};
    ptr += 32;
    return r;
}

[[nodiscard]] constexpr float load_1_float_sample(int8_t const *&ptr, size_t stride) noexcept
{
    float r;
    std::memcpy(&r, ptr, sizeof(float));
    ptr += stride;
    return r;
}


} // namespace tt
