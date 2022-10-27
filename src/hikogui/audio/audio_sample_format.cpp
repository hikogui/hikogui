// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_sample_format.hpp"

namespace hi::inline v1 {

[[nodiscard]] float audio_sample_format::pack_multiplier() const noexcept
{
    if (is_float) {
        return 1.0f;

    } else {
        // Find the maximum value of the fraction bits as a signed number.
        auto max_value = (1_uz << num_bits) - 1;

        // Align left inside an int32_t.
        hi_assert(num_bits + num_guard_bits <= 31);
        max_value <<= narrow_cast<size_t>(31 - num_bits - num_guard_bits);

        return narrow_cast<float>(max_value);
    }
}

[[nodiscard]] float audio_sample_format::unpack_multiplier() const noexcept
{
    return 1.0f / pack_multiplier();
}

[[nodiscard]] std::size_t audio_sample_format::num_samples_per_chunk(std::size_t stride) const noexcept
{
    auto r = narrow_cast<int>(std::bit_floor((((16u - num_bytes) / stride) & 3) + 1));
    hi_assert(r == 1 or r == 2 or r == 4);
    return r;
}

[[nodiscard]] std::size_t audio_sample_format::chunk_stride(std::size_t stride) const noexcept
{
    return stride * num_samples_per_chunk(stride);
}

[[nodiscard]] std::size_t audio_sample_format::num_chunks_per_quad(std::size_t stride) const noexcept
{
    return 4 / num_samples_per_chunk(stride);
}

[[nodiscard]] std::size_t audio_sample_format::num_fast_quads(std::size_t stride, std::size_t num_samples) const noexcept
{
    hilet src_buffer_size = (num_samples - 1) * stride + num_bytes;
    if (src_buffer_size < 16) {
        return 0;
    }

    hilet num_chunks = (src_buffer_size - 16) / chunk_stride(stride) + 1;
    return num_chunks / num_chunks_per_quad(stride);
}

[[nodiscard]] i8x16 audio_sample_format::load_shuffle_indices(std::size_t stride) const noexcept
{
    hilet num_samples = num_samples_per_chunk(stride);

    // Indices set to -1 result in a zero after a byte shuffle.
    auto r = i8x16::broadcast(-1);
    for (int sample_nr = 0; sample_nr != num_samples; ++sample_nr) {
        hilet sample_src_offset = sample_nr * stride;

        // Offset the samples to the highest elements in the i32x4 vector.
        // By shifting the samples from high to low together with 'OR' we can
        // concatenate 1, 2, or 4 loads into a single 4 samples vector.
        // Where the sample in the lowest index is the first sample in memory.
        hilet sample_dst_offset = (sample_nr + (4 - num_samples)) * 4;

        // Bytes are ordered least to most significant.
        for (int byte_nr = 0; byte_nr != num_bytes; ++byte_nr) {
            hilet src_offset = sample_src_offset + (endian == std::endian::little ? byte_nr : num_bytes - byte_nr - 1);

            // Offset the bytes so they become aligned to the left.
            hilet dst_offset = sample_dst_offset + byte_nr + (4 - num_bytes);

            r[dst_offset] = narrow_cast<int8_t>(src_offset);
        }
    }

    return r;
}

[[nodiscard]] i8x16 audio_sample_format::store_shuffle_indices(std::size_t stride) const noexcept
{
    hilet num_samples = num_samples_per_chunk(stride);

    // Indices set to -1 result in a zero after a byte shuffle.
    auto r = i8x16::broadcast(-1);
    for (int sample_nr = 0; sample_nr != num_samples; ++sample_nr) {
        hilet sample_dst_offset = sample_nr * stride;

        // Offset the samples to the lowest elements in the i32x4 vector.
        // By shifting the samples from high to low we can extract 1, 2, or 4 stores
        // from a single 4 samples vector.
        // Where the sample at the lowest index becomes the first sample in memory.
        hilet sample_src_offset = sample_nr * 4;

        // Bytes are ordered least to most significant.
        for (int byte_nr = 0; byte_nr != num_bytes; ++byte_nr) {
            hilet dst_offset = sample_dst_offset + (endian == std::endian::little ? byte_nr : num_bytes - byte_nr - 1);

            // Offset the bytes so they become aligned to the left.
            hilet src_offset = sample_src_offset + byte_nr + (4 - num_bytes);

            r[dst_offset] = narrow_cast<int8_t>(src_offset);
        }
    }

    return r;
}

[[nodiscard]] i8x16 audio_sample_format::concat_shuffle_indices(std::size_t stride) const noexcept
{
    hilet num_samples = num_samples_per_chunk(stride);

    // The bytes are shifted right.
    hilet byte_shift = (4 - num_samples) * 4;

    return i8x16::byte_srl_shuffle_indices(narrow_cast<unsigned int>(byte_shift));
}

} // namespace hi::inline v1
