// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unpack_audio_samples.hpp"
#include "../required.hpp"
#include "../geometry/numeric_array.hpp"
#include <bit>
#include <cstdint>

namespace tt {

[[nodiscard]] tt_force_inline f32x4 load_samples(std::byte const *&src, audio_sample_format src_format, size_t count) noexcept
{
    int align_left = 32 - (src_format.num_bytes * 8);

    auto int_samples = i32x4{};
    for (size_t i = 0; i != count; ++i) {
        uint32_t u32 = 0;
        if (src_format.endian == std::endian::little) {
            for (size_t j = 0; j != src_format.num_bytes; ++j) {
                u32 >>= 8;
                u32 |= static_cast<uint32_t>(*(src++)) << 24;
            }
            int_samples[i] = static_cast<int32_t>(u32);

        } else {
            for (size_t j = 0; j != src_format.num_bytes; ++j) {
                u32 <<= 8;
                u32 |= static_cast<uint32_t>(*(src++));
            }
            int_samples[i] = static_cast<int32_t>(u32 << align_left);
        }

        // Skip over the other channels.
        src += (src_format.num_channels - 1) * src_format.num_bytes;
    }

    auto float_samples = (src_format.numeric_type == audio_sample_format::numeric_type::floating_point) ?
        std::bit_cast<f32x4>(int_samples) :
        static_cast<f32x4>(int_samples);

    return float_samples;
}

tt_force_inline void unpack_audio_samples_actual(
    std::byte const *tt_restrict &src,
    audio_sample_format src_format,
    float *tt_restrict &dst,
    size_t quad_count,
    unpack_audio_samples_context &context) noexcept
{
    for (size_t i = 0; i != quad_count; ++i) {
        auto quad = load_samples(src, src_format, 4);
        quad *= context.gain;

        // measure_loudness(quad, context);
        std::memcpy(dst, &quad, sizeof(quad));
        dst += 4;
    }
}

tt_force_inline void unpack_audio_samples_num_channels(
    std::byte const *tt_restrict &src,
    audio_sample_format src_format,
    float *tt_restrict &dst,
    size_t quad_count,
    unpack_audio_samples_context &context)
{
    if (src_format.num_channels == 1) {
        return unpack_audio_samples_actual(src, src_format, dst, quad_count, context);
    } else if (src_format.num_channels == 2) {
        return unpack_audio_samples_actual(src, src_format, dst, quad_count, context);
    } else if (src_format.num_channels == 4) {
        return unpack_audio_samples_actual(src, src_format, dst, quad_count, context);
    } else {
        return unpack_audio_samples_actual(src, src_format, dst, quad_count, context);
    }
}

tt_force_inline void unpack_audio_samples_numeric_type(
    std::byte const *tt_restrict &src,
    audio_sample_format src_format,
    float *tt_restrict &dst,
    size_t quad_count,
    unpack_audio_samples_context &context)
{
    if (src_format.numeric_type == audio_sample_format::numeric_type::floating_point) {
        return unpack_audio_samples_num_channels(src, src_format, dst, quad_count, context);
    } else {
        return unpack_audio_samples_num_channels(src, src_format, dst, quad_count, context);
    }
}

tt_force_inline void unpack_audio_samples_num_bytes(
    std::byte const *tt_restrict &src,
    audio_sample_format src_format,
    float *tt_restrict &dst,
    size_t quad_count,
    unpack_audio_samples_context &context)
{
    if (src_format.num_bytes == 1) {
        return unpack_audio_samples_numeric_type(src, src_format, dst, quad_count, context);
    } else if (src_format.num_bytes == 2) {
        return unpack_audio_samples_numeric_type(src, src_format, dst, quad_count, context);
    } else if (src_format.num_bytes == 3) {
        return unpack_audio_samples_numeric_type(src, src_format, dst, quad_count, context);
    } else if (src_format.num_bytes == 4) {
        return unpack_audio_samples_numeric_type(src, src_format, dst, quad_count, context);
    } else {
        tt_not_implemented();
    }
}

tt_force_inline void unpack_audio_samples_endian(
    std::byte const *tt_restrict &src,
    audio_sample_format src_format,
    float *tt_restrict &dst,
    size_t quad_count,
    unpack_audio_samples_context &context)
{
    if (src_format.endian == std::endian::little) {
        return unpack_audio_samples_num_bytes(src, src_format, dst, quad_count, context);
    } else if (src_format.endian == std::endian::big) {
        return unpack_audio_samples_num_bytes(src, src_format, dst, quad_count, context);
    } else {
        tt_not_implemented();
    }
}

void unpack_audio_samples(
    std::byte const *tt_restrict src,
    audio_sample_format src_format,
    float *tt_restrict dst,
    size_t count,
    unpack_audio_samples_context &context)
{
    ttlet quad_count = count / 4;
    ttlet last_count = count % 4;

    auto context_ = context;
    context_.gain = f32x4::broadcast(src_format.unpack_gain());

    unpack_audio_samples_endian(src, src_format, dst, quad_count, context_);

    auto quad = load_samples(src, src_format, last_count);
    quad *= context_.gain;
    //measure_loudness(quad, last_count);
    std::memcpy(dst, &quad, sizeof(quad));

    context = context_;
}

} // namespace tt
