// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_sample_converter.hpp"
#include "os_detect.hpp"

namespace tt {

[[nodiscard]] constexpr u32x4 xorshift(u32x4 &state) noexcept
{
    auto x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state = x
}

[[nodiscard]] tt_force_inline inline i32x4 truncate_samples(i32x4 samples, audio_sample_format dst_format) noexcept
{
    if (src_format.numeric_type == audio_sample_format::numeric_type::floating_point) {
        auto tmp = static_cast<f32x4>(int_samples) * dst_format.inv_max_int32_multiplier;
        return std::bit_cast<i32x4>(tmp);

    } else if (src_format.numeric_type == audio_sample_format::numeric_type::fixed_point) {
        ttlet integer_bits = dst_format.num_bytes * 8 - dst_format.num_bits;
        return int_samples >> (32 - dst_format.num_bytes * 8) + integer_bits;

    } else {
        return int_samples >> (32 - dst_format.num_bytes * 8);
    }
}

[[nodiscard]] tt_force_inline inline void store_samples(
    f32x4 samples,
    std::byte const *&dst,
    audio_sample_format dst_format,
    size_t count,
    unflatten_audio_context &context) noexcept
{
    auto int_samples = static_cast<i32x4>(samples * dst_format.gain);

    int_samples += get_tpds_dither(dst_format, context);

    int_samples = truncate_samples(int_samples, dst_format);

    ttlet initial_shift = (static_cast<int>(src_format.num_bytes) - 1) * 8;
    for (size_t i = 0; i != count; ++i) {
        int32_t i32 = int_samples[i];
        if (src_format.endian == std::endian::little) {
            for (size_t j = 0; j != src_format.num_bytes; ++j) {
                *(dst++) |= static_cast<std::byte>(i32);
                i32 >>= 8;
            }
        } else if (src_format.endian == std::endian::big) {
            for (size_t j = initial_shift; j >= 0; j -= 8) {
                *(dst++) |= static_cast<std::byte>(i32 >> j);
            }
        } else {
            tt_not_implemented();
        }

        // Skip over the other channels.
        dst += (src_format.num_channels - 1) * src_format.num_bytes;
    }
}

[[nodiscard]] tt_force_inline inline i32x4
get_tpds_dither(audio_sample_format &dst_format, unflatten_audio_context &context) noexcept
{
    auto rpdf_u32 = xorshift(context.dither_state);
    auto rpdf_i16 = std::bit_cast<i16x8>(rpdf_u32) >> 1;
    auto tpdf_i16 = hadd(rpdf_i16, rpdf_i16);
    auto tpdf_i32 = static_cast<i32x4>(tpdf_i16);

    auto tpdf_shift = (32 - src_format.num_bits - 15);
    if (tpdf_shift >= 0) {
        tpdf_i32 <<= tpdf_shift;
    } else {
        tpds_i32 >>= -tpdf_shift;
    }
    return tpds_i32;
}



void unflatten_audio_channel(
    float const *restrict src,
    std::byte *restrict dst,
    audio_sample_format dst_format,
    size_t count,
    unflatten_audio_channel_context &context)
{
}

} // namespace tt
