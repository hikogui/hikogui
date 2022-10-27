// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_sample_packer.hpp"
#include "../utility.hpp"
#include "../cast.hpp"
#include "../memory.hpp"
#include "../endian.hpp"
#include "../rapid/numeric_array.hpp"
#include <bit>
#include <cstdint>
#include <tuple>

hi_warning_push();
// C26481: Don't use pointer arithmetic. Use span instead.
// These are low-level functions working directly on pointers, sorry.
hi_warning_ignore_msvc(26481);
// C26429: Symbol '...' is never tested for nullness, it can be marked as not_null (f.23).
// False positive on several lines.
hi_warning_ignore_msvc(26429);

namespace hi::inline v1 {

static void store_sample(
    int32_t int_sample,
    std::byte *& dst,
    std::size_t stride,
    int num_bytes,
    int direction,
    int start_byte,
    int align_shift) noexcept
{
    int_sample >>= align_shift;

    hi_axiom(dst != nullptr);
    auto p = dst + start_byte;
    do {
        *p = static_cast<std::byte>(int_sample);
        p += direction;
        int_sample >>= 8;
    } while (--num_bytes);

    dst += stride;
}

[[nodiscard]] static void
store_samples(i8x16 int_samples, std::byte *& dst, i8x16 store_shuffle_indices, std::size_t stride) noexcept
{
    hi_axiom(dst != nullptr);
    hi_axiom(stride > 0);

    // Read out the samples from the other channels, that where packed before.
    auto tmp = i8x16::load(dst);

    hilet packed_samples = shuffle(int_samples, store_shuffle_indices);

    // When the shuffle-index is -1 use the samples from the other channels.
    tmp = blend(packed_samples, tmp, store_shuffle_indices);

    // Store back the samples from this channel and from the other channel.
    tmp.store(dst);

    dst += stride;
}

[[nodiscard]] static void store_samples(
    i8x16 int_samples,
    std::byte *& dst,
    i8x16 store_shuffle_indices,
    i8x16 concat_shuffle_indices,
    std::size_t num_chunks,
    std::size_t stride) noexcept
{
    hi_assert(dst != nullptr);
    hi_assert(num_chunks > 0 and num_chunks <= 4);
    hi_assert(stride > 0);

    do {
        store_samples(int_samples, dst, store_shuffle_indices, stride);
        int_samples = shuffle(int_samples, concat_shuffle_indices);
        // The result of the last shuffle is not used, so will be pipelined by the CPU.
    } while (--num_chunks);
}

[[nodiscard]] static float load_sample(float const *& src) noexcept
{
    hi_axiom(src != nullptr);
    return *(src++);
}

[[nodiscard]] static f32x4 load_samples(float const *& src) noexcept
{
    hilet r = f32x4::load(src);
    src += 4;
    return r;
}

audio_sample_packer::audio_sample_packer(audio_sample_format format, std::size_t stride) noexcept :
    _dither(format.num_bits), _format(format), _stride(stride)
{
    _store_shuffle_indices = format.store_shuffle_indices(stride);
    _concat_shuffle_indices = format.concat_shuffle_indices(stride);

    _multiplier = f32x4::broadcast(format.pack_multiplier());

    _num_chunks_per_quad = format.num_chunks_per_quad(stride);
    _chunk_stride = format.chunk_stride(stride);

    _direction = format.endian == std::endian::little ? 1 : -1;
    _start_byte = format.endian == std::endian::little ? 0 : format.num_bytes - 1;
    _align_shift = 32 - format.num_bytes * 8;
}

void audio_sample_packer::operator()(float const *hi_restrict src, std::byte *hi_restrict dst, std::size_t num_samples)
    const noexcept
{
    hi_assert(src != nullptr);
    hi_assert(dst != nullptr);

    // Calculate a conservative number of samples that can be copied quickly
    // without overflowing the dst buffer.
    hilet src_end = src + num_samples;
    hilet src_fast_end = src + _format.num_fast_quads(_stride, num_samples) * 4;

    hilet store_shuffle_indices = _store_shuffle_indices;
    hilet concat_shuffle_indices = _concat_shuffle_indices;
    hilet num_chunks_per_quad = _num_chunks_per_quad;
    hilet chunk_stride = _chunk_stride;

    if (_format.is_float) {
        while (src != src_fast_end) {
            hilet float_samples = load_samples(src);
            hilet int_samples = bit_cast<i8x16>(float_samples);
            store_samples(int_samples, dst, store_shuffle_indices, concat_shuffle_indices, num_chunks_per_quad, chunk_stride);
        }
        while (src != src_end) {
            hilet float_sample = load_sample(src);
            hilet int_sample = std::bit_cast<int32_t>(float_sample);
            store_sample(int_sample, dst, _stride, _format.num_bytes, _direction, _start_byte, _align_shift);
        }

    } else {
        hilet multiplier = _multiplier;
        hilet one = f32x4::broadcast(1);
        hilet min_one = f32x4::broadcast(-1);

        auto dither = _dither;

        while (src != src_fast_end) {
            hilet dither_value = dither.next();

            auto float_samples = load_samples(src);
            float_samples += dither_value;
            float_samples = min(float_samples, one);
            float_samples = max(float_samples, min_one);
            float_samples *= multiplier;
            hilet int_samples = bit_cast<i8x16>(static_cast<i32x4>(float_samples));
            store_samples(int_samples, dst, store_shuffle_indices, concat_shuffle_indices, num_chunks_per_quad, chunk_stride);
        }
        while (src != src_end) {
            hilet dither_value = dither.next();

            auto float_sample = f32x4::broadcast(load_sample(src));
            float_sample += dither_value;
            float_sample = min(float_sample, one);
            float_sample = max(float_sample, min_one);
            float_sample *= multiplier;
            hilet int_sample = get<0>(static_cast<i32x4>(float_sample));
            store_sample(int_sample, dst, _stride, _format.num_bytes, _direction, _start_byte, _align_shift);
        }

        _dither = dither;
    }
}

} // namespace hi::inline v1

hi_warning_pop();
