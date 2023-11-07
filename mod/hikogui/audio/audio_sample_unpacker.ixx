// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstddef>
#include <bit>

export module hikogui_audio_audio_sample_unpacker;
import hikogui_SIMD;
import hikogui_audio_audio_sample_format;
import hikogui_utility;

export namespace hi { inline namespace v1 {

export class audio_sample_unpacker {
public:
    /** Audio sample unpacker
     * One instance of this class can be used to unpack multiple buffers either
     * from one audio-proc to the next, or for each channel in a group of
     * interleaved channels.
     *
     * @param format The sample format.
     * @param stride The distance to the next sample.
     */
    audio_sample_unpacker(audio_sample_format format, std::size_t stride) noexcept : _format(format), _stride(stride)
    {
        _load_shuffle_indices = format.load_shuffle_indices(stride);
        _concat_shuffle_indices = format.concat_shuffle_indices(stride);

        _multiplier = f32x4::broadcast(format.unpack_multiplier());
        _num_chunks_per_quad = format.num_chunks_per_quad(stride);
        _chunk_stride = format.chunk_stride(stride);

        _direction = format.endian == std::endian::little ? -1 : 1;
        _start_byte = format.endian == std::endian::little ? format.num_bytes - 1 : 0;
        _align_shift = 32 - format.num_bytes * 8;
    }

    /** Unpack samples.
     *
     * @param src A pointer to a byte array containing samples.
     * @param dst A pointer to a array of floating point samples of a single channel.
     * @param num_samples Number of samples.
     */
    void operator()(std::byte const *hi_restrict src, float *hi_restrict dst, std::size_t num_samples) const noexcept
    {
        hi_assert(src != nullptr);
        hi_assert(dst != nullptr);

        // Calculate a conservative number of samples that can be copied quickly
        // without overflowing the src buffer.
        auto const *const dst_end = dst + num_samples;
        auto const *const dst_fast_end = dst + _format.num_fast_quads(_stride, num_samples) * 4;

        if (_format.is_float) {
            while (dst != dst_fast_end) {
                hilet int_samples =
                    load_samples(src, _load_shuffle_indices, _concat_shuffle_indices, _num_chunks_per_quad, _chunk_stride);
                hilet float_samples = f32x4::cast_from(int_samples);
                store_samples(dst, float_samples);
            }
            while (dst != dst_end) {
                hilet int_sample = load_sample(src, _stride, _format.num_bytes, _direction, _start_byte, _align_shift);
                hilet float_sample = std::bit_cast<float>(int_sample);
                store_sample(dst, float_sample);
            }

        } else {
            hilet multiplier = _multiplier;
            while (dst != dst_fast_end) {
                hilet int_samples =
                    load_samples(src, _load_shuffle_indices, _concat_shuffle_indices, _num_chunks_per_quad, _chunk_stride);
                hilet float_samples = static_cast<f32x4>(int_samples) * multiplier;
                store_samples(dst, float_samples);
            }
            while (dst != dst_end) {
                hilet int_sample = load_sample(src, _stride, _format.num_bytes, _direction, _start_byte, _align_shift);
                hilet float_sample = static_cast<float>(int_sample) * get<0>(multiplier);
                store_sample(dst, float_sample);
            }
        }
    }

private:
    f32x4 _multiplier;
    i8x16 _load_shuffle_indices;
    i8x16 _concat_shuffle_indices;
    std::size_t _num_chunks_per_quad;
    std::size_t _stride;
    std::size_t _chunk_stride;
    audio_sample_format _format;
    int _direction;
    int _start_byte;
    int _align_shift;

    //[[nodiscard]] std::size_t calculate_num_fast_samples(std::size_t num_samples) const noexcept;

    [[nodiscard]] static int32_t load_sample(
        std::byte const *hi_restrict & src,
        std::size_t stride,
        int num_bytes,
        int direction,
        int start_byte,
        int align_shift) noexcept
    {
        hi_axiom(src != nullptr);
        hi_axiom(num_bytes >= 1 && num_bytes <= 4);
        hi_axiom(direction == 1 || direction == -1);
        hi_axiom(start_byte <= 3);
        hi_axiom(align_shift <= 32);
        hi_axiom(stride >= num_bytes);

        auto p = src + start_byte;
        hi_axiom(p != nullptr);

        uint32_t r = 0;
        do {
            r <<= 8;
            r |= static_cast<uint32_t>(static_cast<uint8_t>(*p));
            p += direction;
        } while (--num_bytes);

        // Align the bits to the left to allow for sign extension.
        r <<= align_shift;

        src += stride;
        return truncate<int32_t>(r);
    }

    [[nodiscard]] static i8x16 load_samples(std::byte const *hi_restrict & src, i8x16 load_shuffle_indices, std::size_t stride) noexcept
    {
        hi_axiom(src != nullptr);
        hi_axiom(stride > 0);

        auto r = permute(i8x16::load(src), load_shuffle_indices);
        src += stride;
        return r;
    }

    [[nodiscard]] static i32x4 load_samples(
        std::byte const *hi_restrict & src,
        i8x16 load_shuffle_indices,
        i8x16 concat_shuffle_indices,
        std::size_t num_chunks,
        std::size_t stride) noexcept
    {
        hi_assert(src != nullptr);
        hi_assert(num_chunks > 0 and num_chunks <= 4);
        hi_assert(stride > 0);

        auto int_samples = i8x16{};
        do {
            int_samples = permute(int_samples, concat_shuffle_indices);
            // Due to int_samples reset the dependency is broken on the first iteration, the load_samples
            // call here should be pipelined in parallel with the first shuffle.
            int_samples |= load_samples(src, load_shuffle_indices, stride);
        } while (--num_chunks);

        return std::bit_cast<i32x4>(int_samples);
    }

    static void store_sample(float *hi_restrict & dst, float sample) noexcept
    {
        hi_axiom(dst != nullptr);
        *(dst++) = sample;
    }

    static void store_samples(float *hi_restrict & dst, f32x4 samples) noexcept
    {
        hi_axiom(dst != nullptr);
        samples.store(reinterpret_cast<std::byte *>(dst));
        dst += 4;
    }
};

}} // namespace hi::inline v1
