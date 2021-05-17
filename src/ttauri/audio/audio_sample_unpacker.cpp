// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_sample_unpacker.hpp"
#include "../required.hpp"
#include "../cast.hpp"
#include "../memory.hpp"
#include "../endian.hpp"
#include "../rapid/numeric_array.hpp"
#include <bit>
#include <cstdint>
#include <tuple>

namespace tt {

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

[[nodiscard]] static i8x16 load_samples(std::byte const *&src, i8x16 load_shuffle_indices, int stride) noexcept
{
    tt_axiom(src != nullptr);
    tt_axiom(stride > 0);

    auto r = shuffle(i8x16::load(src), load_shuffle_indices);
    src += stride;
    return r;
}

[[nodiscard]] static i32x4 load_samples(
    std::byte const *&src,
    i8x16 load_shuffle_indices,
    i8x16 concat_shuffle_indices,
    int num_chunks,
    int stride) noexcept
{
    tt_axiom(src != nullptr);
    tt_axiom(num_chunks > 0 and num_chunks <= 4);
    tt_axiom(stride > 0);

    auto int_samples = load_samples(src, load_shuffle_indices, stride);

    while (--num_chunks) {
        int_samples = shuffle(int_samples, concat_shuffle_indices);
        int_samples |= load_samples(src, load_shuffle_indices, stride);
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

audio_sample_unpacker::audio_sample_unpacker(audio_sample_format format) noexcept : _format(format)
{
    _load_shuffle_indices = format.unpack_load_shuffle_indices();
    _concat_shuffle_indices = format.unpack_concat_shuffle_indices();

    _multiplier = f32x4::broadcast(format.unpack_multiplier());
    _num_chunks_per_quad = format.num_chunks_per_quad();
    _chunk_stride = format.chunk_stride();

    _direction = format.endian == std::endian::little ? -1 : 1;
    _start_byte = format.endian == std::endian::little ? format.num_bytes - 1 : 0;
    _align_shift = 32 - format.num_bytes * 8;
}



void audio_sample_unpacker::operator()(std::byte const *tt_restrict src, float *tt_restrict dst, size_t num_samples)
    const noexcept
{
    tt_axiom(src != nullptr);
    tt_axiom(dst != nullptr);
    tt_axiom(_format.is_valid());

    // Calculate a conservative number of samples that can be copied quickly
    // without overflowing the src buffer.
    ttlet dst_end = dst + num_samples;
    ttlet dst_fast_end = dst + _format.num_fast_quads(num_samples) * 4;

    if (_format.is_float) {
        while (dst != dst_fast_end) {
            ttlet int_samples =
                load_samples(src, _load_shuffle_indices, _concat_shuffle_indices, _num_chunks_per_quad, _chunk_stride);
            ttlet float_samples = bit_cast<f32x4>(int_samples);
            store_samples(dst, float_samples);
        }
        while (dst != dst_end) {
            ttlet int_sample = load_sample(src, _format.stride, _format.num_bytes, _direction, _start_byte, _align_shift);
            ttlet float_sample = std::bit_cast<float>(int_sample);
            store_sample(dst, float_sample);
        }

    } else {
        ttlet multiplier = _multiplier;

        while (dst != dst_fast_end) {
            ttlet int_samples =
                load_samples(src, _load_shuffle_indices, _concat_shuffle_indices, _num_chunks_per_quad, _chunk_stride);
            ttlet float_samples = static_cast<f32x4>(int_samples) * multiplier;
            store_samples(dst, float_samples);
        }
        while (dst != dst_end) {
            ttlet int_sample = load_sample(src, _format.stride, _format.num_bytes, _direction, _start_byte, _align_shift);
            ttlet float_sample = static_cast<float>(int_sample) * get<0>(multiplier);
            store_sample(dst, float_sample);
        }
    }
}

} // namespace tt
