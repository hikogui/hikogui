// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_sample_format.hpp"

namespace tt {

[[nodiscard]] float audio_sample_format::pack_multiplier() const noexcept
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

[[nodiscard]] float audio_sample_format::unpack_multiplier() const noexcept
{
    return 1.0f / pack_multiplier();
}

[[nodiscard]] int audio_sample_format::num_samples_per_chunk() const noexcept
{
    tt_axiom(is_valid());

    auto r = narrow_cast<int>(std::bit_floor((((16u - num_bytes) / stride) & 3) + 1));
    tt_axiom(r == 1 || r == 2 || r == 4);
    return r;
}

[[nodiscard]] int audio_sample_format::chunk_stride() const noexcept
{
    return stride * num_samples_per_chunk();
}

[[nodiscard]] int audio_sample_format::num_chunks_per_quad() const noexcept
{
    return 4 / num_samples_per_chunk();
}

[[nodiscard]] size_t audio_sample_format::num_fast_quads(size_t num_samples) const noexcept
{
    tt_axiom(is_valid());

    ttlet src_buffer_size = (num_samples - 1) * stride + num_bytes;
    if (src_buffer_size < 16) {
        return 0;
    }

    auto num_chunks = (src_buffer_size - 16) / chunk_stride() + 1;
    return num_chunks / num_chunks_per_quad();
}

[[nodiscard]] i8x16 audio_sample_format::unpack_load_shuffle_indices() const noexcept
{
    tt_axiom(is_valid());
    ttlet num_samples = num_samples_per_chunk();

    // Indices set to -1 result in a zero after a byte shuffle.
    auto r = i8x16::broadcast(-1);
    for (int sample_nr = 0; sample_nr != num_samples; ++sample_nr) {
        ttlet sample_src_offset = sample_nr * stride;

        // Offset the samples to the highest elements in the i32x4 vector.
        // By shifting the samples from high to low together with 'OR' we can
        // concatenate 1, 2, or 4 loads into a single 4 samples vector.
        // Where the sample in the lowest index is the first sample in memory.
        ttlet sample_dst_offset = (sample_nr + (4 - num_samples)) * 4;

        // Bytes are ordered least to most significant.
        for (int byte_nr = 0; byte_nr != num_bytes; ++byte_nr) {
            ttlet src_offset = sample_src_offset + (endian == std::endian::little ? byte_nr : num_bytes - byte_nr - 1);

            // Offset the bytes so they become aligned to the left.
            ttlet dst_offset = sample_dst_offset + byte_nr + (4 - num_bytes);

            r[dst_offset] = narrow_cast<int8_t>(src_offset);
        }
    }

    return r;
}

[[nodiscard]] i8x16 audio_sample_format::pack_store_shuffle_indices() const noexcept
{
    tt_axiom(is_valid());
    ttlet num_samples = num_samples_per_chunk();

    // Indices set to -1 result in a zero after a byte shuffle.
    auto r = i8x16::broadcast(-1);
    for (int sample_nr = 0; sample_nr != num_samples; ++sample_nr) {
        ttlet sample_dst_offset = sample_nr * stride;

        // Offset the samples to the lowest elements in the i32x4 vector.
        // By shifting the samples from high to low we can extract 1, 2, or 4 stores
        // from a single 4 samples vector.
        // Where the sample at the lowest index becomes the first sample in memory.
        ttlet sample_src_offset = sample_nr * 4;

        // Bytes are ordered least to most significant.
        for (int byte_nr = 0; byte_nr != num_bytes; ++byte_nr) {
            ttlet dst_offset = sample_dst_offset + (endian == std::endian::little ? byte_nr : num_bytes - byte_nr - 1);

            // Offset the bytes so they become aligned to the left.
            ttlet src_offset = sample_src_offset + byte_nr + (4 - num_bytes);

            r[dst_offset] = narrow_cast<int8_t>(src_offset);
        }
    }

    return r;
}

[[nodiscard]] i8x16 audio_sample_format::unpack_concat_shuffle_indices() const noexcept
{
    tt_axiom(is_valid());
    ttlet num_samples = num_samples_per_chunk();

    // The bytes are shifted right.
    ttlet byte_shift = (4 - num_samples) * 4;

    return i8x16::byte_srl_shuffle_indices(byte_shift);
}

[[nodiscard]] i8x16 audio_sample_format::pack_split_shuffle_indices() const noexcept
{
    return unpack_concat_shuffle_indices();
}

[[nodiscard]] bool audio_sample_format::is_valid() const noexcept
{
    return (num_bytes >= 1 && num_bytes <= 4) && (num_bits + num_guard_bits <= num_bytes * 8) &&
        (endian == std::endian::little || endian == std::endian::big) && (stride >= num_bytes);
}

} // namespace tt
