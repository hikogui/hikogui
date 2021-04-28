// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_sample_unpacker.hpp"
#include "../required.hpp"
#include "../cast.hpp"
#include "../memory.hpp"
#include "../endian.hpp"
#include "../geometry/numeric_array.hpp"
#include <bit>
#include <cstdint>
#include <tuple>

namespace tt {

[[nodiscard]] static size_t calculate_num_fast_samples(size_t num_samples, audio_sample_format format) noexcept
{
    tt_axiom(format.is_valid());
    ttlet num_samples_per_load = format.samples_per_load();
    ttlet num_loads_per_store = format.loads_per_store();
    ttlet load_stride = format.load_stride();

    ttlet src_buffer_size = (num_samples - 1) * format.stride + format.num_bytes;

    auto num_loads = num_samples / num_samples_per_load;
    while ((num_loads > 0) and ((num_loads - 1) * load_stride + 16 > src_buffer_size)) {
        --num_loads;
    }
    num_loads = floor(num_loads, static_cast<size_t>(num_loads_per_store));

    return num_loads * num_samples_per_load;
}

[[nodiscard]] static i8x16 make_shuffle_load(audio_sample_format format) noexcept
{
    tt_axiom(format.is_valid());
    ttlet num_samples = format.samples_per_load();

    // Indices set to -1 result in a zero after a byte shuffle.
    auto r = i8x16::broadcast(-1);
    for (int sample_nr = 0; sample_nr != num_samples; ++sample_nr) {
        ttlet sample_src_offset = sample_nr * format.stride;

        // Offset the samples to the right side, so we can do left-shift-or to combine multiple loads.
        ttlet sample_dst_offset = (sample_nr + (4 - num_samples)) * 4;

        // Bytes are ordered least to most significant.
        for (int byte_nr = 0; byte_nr != format.num_bytes; ++byte_nr) {
            ttlet src_offset =
                sample_src_offset + (format.endian == std::endian::little ? byte_nr : format.num_bytes - byte_nr - 1);

            // Offset the bytes so they become aligned to the left.
            ttlet dst_offset = sample_dst_offset + byte_nr + (4 - format.num_bytes);

            r[dst_offset] = narrow_cast<int8_t>(src_offset);
        }
    }

    return r;
}

[[nodiscard]] static i8x16 make_shuffle_shift(audio_sample_format format) noexcept
{
    tt_axiom(format.is_valid());
    ttlet num_samples = format.samples_per_load();

    // The bytes are shifted right.
    ttlet byte_shift = (4 - num_samples) * 4;

    // Indices set to -1 result in a zero after a byte shuffle.
    i8x16 r;
    for (auto i = 0; i != 16; ++i) {
        if ((i + byte_shift) < 16) {
            r[i] = narrow_cast<int8_t>(i + byte_shift);
        } else {
            r[i] = -1;
        }
    }
    return r;
}

audio_sample_unpacker::audio_sample_unpacker(audio_sample_format format) noexcept : _format(format)
{
    _shuffle_load = make_shuffle_load(format);
    _shuffle_shift = make_shuffle_shift(format);
}

[[nodiscard]] static int32_t
load_sample(std::byte const *&src, int stride, int num_bytes, int direction, int start_byte, int align_shift) noexcept
{
    tt_axiom(src != nullptr);
    tt_axiom(num_bytes >= 1 && num_bytes <= 4);
    tt_axiom(direction == 1 || direction == -1);
    tt_axiom(start_byte <= 3);
    tt_axiom(align_shift <= 32);
    tt_axiom(stride >= num_bytes);

    auto p = src + start_byte;

    uint32_t r = 0;
    do {
        r <<= 8;
        r |= static_cast<uint32_t>(static_cast<uint8_t>(*p));
        p += direction;
    } while (--num_bytes);

    // Align the bits to the left to allow for sign extension.
    r <<= align_shift;

    src += stride;
    return static_cast<int32_t>(r);
}

[[nodiscard]] static i8x16 load_samples(std::byte const *&src, i8x16 shuffle_load, int load_stride) noexcept
{
    tt_axiom(src != nullptr);
    tt_axiom(load_stride > 0);

    auto r = shuffle(i8x16::load(src), shuffle_load);
    src += load_stride;
    return r;
}

[[nodiscard]] static i32x4
load_samples(std::byte const *&src, i8x16 shuffle_load, i8x16 shuffle_shift, int num_loads, int load_stride) noexcept
{
    tt_axiom(src != nullptr);
    tt_axiom(num_loads > 0 and num_loads <= 4);
    tt_axiom(load_stride > 0);

    auto int_samples = load_samples(src, shuffle_load, load_stride);

    while (--num_loads) {
        int_samples = shuffle(int_samples, shuffle_shift);
        int_samples |= load_samples(src, shuffle_load, load_stride);
    };

    return std::bit_cast<i32x4>(int_samples);
}

static void store_sample(float *&dst, float sample) noexcept
{
    tt_axiom(dst != nullptr);
    *(dst++) = sample;
}

static void store_samples(float *&dst, f32x4 samples) noexcept
{
    tt_axiom(dst != nullptr);
    samples.store(reinterpret_cast<std::byte *>(dst));
    dst += 4;
}

void audio_sample_unpacker::operator()(std::byte const *tt_restrict src, float *tt_restrict dst, size_t num_samples)
    const noexcept
{
    tt_axiom(src != nullptr);
    tt_axiom(dst != nullptr);

    ttlet format = _format;
    tt_axiom(format.is_valid());

    // Calculate a conservative number of samples that can be copied quickly
    // without overflowing the src buffer.
    ttlet dst_end = dst + num_samples;
    ttlet dst_fast_end = dst + calculate_num_fast_samples(num_samples, format);

    // Load variables outside the loop.
    ttlet gain = f32x4::broadcast(format.unpack_gain());
    ttlet num_loads_per_store = format.loads_per_store();
    ttlet load_stride = format.load_stride();

    ttlet direction = format.endian == std::endian::little ? -1 : 1;
    ttlet start_byte = format.endian == std::endian::little ? format.num_bytes - 1 : 0;
    ttlet align_shift = 32 - format.num_bytes * 8;

    if (format.is_float) {
        while (dst != dst_fast_end) {
            ttlet int_samples = load_samples(src, _shuffle_load, _shuffle_shift, num_loads_per_store, load_stride);
            ttlet float_samples = bit_cast<f32x4>(int_samples);
            store_samples(dst, float_samples);
        }
        while (dst != dst_end) {
            ttlet int_sample = load_sample(src, format.stride, format.num_bytes, direction, start_byte, align_shift);
            ttlet float_sample = std::bit_cast<float>(int_sample);
            store_sample(dst, float_sample);
        }

    } else {
        while (dst != dst_fast_end) {
            ttlet int_samples = load_samples(src, _shuffle_load, _shuffle_shift, num_loads_per_store, load_stride);
            ttlet float_samples = static_cast<f32x4>(int_samples) * gain;
            store_samples(dst, float_samples);
        }
        while (dst != dst_end) {
            ttlet int_sample = load_sample(src, format.stride, format.num_bytes, direction, start_byte, align_shift);
            ttlet float_sample = static_cast<float>(int_sample) * get<0>(gain);
            store_sample(dst, float_sample);
        }
    }
}

} // namespace tt
