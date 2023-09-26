// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_sample_format.hpp"
#include "../utility/utility.hpp"
#include "../SIMD/module.hpp"
#include "../random/random.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <bit>

hi_export_module(hikogui.audio.audio_sample_packer);

namespace hi { inline namespace v1 {

hi_export class audio_sample_packer {
public:
    /** Audio sample packer
     * One instance of this class can be used to pack multiple buffers either
     * from one audio-proc to the next, or for each channel in a group of
     * interleaved channels.
     *
     * @param format The sample format.
     * @param stride Number of bytes to step for the next sample of the same channel.
     */
    audio_sample_packer(audio_sample_format format, std::size_t stride) noexcept :
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

    /** Unpack samples.
     *
     * @param src A pointer to an array of floating point samples of a single channel.
     * @param dst A pointer to a byte array to store the packed samples into.
     * @param num_samples Number of samples.
     */
    void operator()(float const *hi_restrict src, std::byte *hi_restrict dst, std::size_t num_samples) const noexcept
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
                hilet int_samples = i8x16::cast_from(float_samples);
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
                hilet int_samples = i8x16::cast_from(static_cast<i32x4>(float_samples));
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

private:
    i8x16 _store_shuffle_indices;
    i8x16 _concat_shuffle_indices;
    f32x4 _multiplier;
    mutable dither _dither;
    audio_sample_format _format;
    std::size_t _num_chunks_per_quad;
    std::size_t _stride;
    std::size_t _chunk_stride;
    int _direction;
    int _start_byte;
    int _align_shift;

    static void store_sample(
        int32_t int_sample,
        std::byte * hi_restrict & dst,
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

    static void
    store_samples(i8x16 int_samples, std::byte * hi_restrict & dst, i8x16 store_shuffle_indices, std::size_t stride) noexcept
    {
        hi_axiom(dst != nullptr);
        hi_axiom(stride > 0);

        // Read out the samples from the other channels, that where packed before.
        auto tmp = i8x16::load(dst);

        hilet packed_samples = permute(int_samples, store_shuffle_indices);

        // When the shuffle-index is -1 use the samples from the other channels.
        tmp = blend(packed_samples, tmp, store_shuffle_indices);

        // Store back the samples from this channel and from the other channel.
        tmp.store(dst);

        dst += stride;
    }

    static void store_samples(
        i8x16 int_samples,
        std::byte * hi_restrict & dst,
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
            int_samples = permute(int_samples, concat_shuffle_indices);
            // The result of the last shuffle is not used, so will be pipelined by the CPU.
        } while (--num_chunks);
    }

    [[nodiscard]] static float load_sample(float const * hi_restrict & src) noexcept
    {
        hi_axiom(src != nullptr);
        return *(src++);
    }

    [[nodiscard]] static f32x4 load_samples(float const * hi_restrict & src) noexcept
    {
        hilet r = f32x4::load(src);
        src += 4;
        return r;
    }
};

}} // namespace hi::inline v1
