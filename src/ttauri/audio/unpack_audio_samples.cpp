// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unpack_audio_samples.hpp"
#include "../required.hpp"
#include "../cast.hpp"
#include "../memory.hpp"
#include "../endian.hpp"
#include "../geometry/numeric_array.hpp"
#include <bit>
#include <cstdint>
#include <tuple>

namespace tt {

[[nodiscard]] static int calculate_num_samples_per_load(int num_bytes, int stride) noexcept
{
    ttlet bytes_after_first_sample = 16 - num_bytes;
    switch (bytes_after_first_sample / stride + 1) {
    case 1: return 1;
    case 2: return 2;
    case 3: return 2; // Only allow power of 2 loads.
    case 4: return 4;
    default: return 4;
    }
}

[[nodiscard]] static i8x16 make_shuffle_load(int num_bytes, std::endian endian, int stride, int num_samples) noexcept
{
    // Indices set to -1 result in a zero after a byte shuffle.
    auto r = i8x16::broadcast(-1);
    for (int sample_nr = 0; sample_nr != num_samples; ++sample_nr) {
        ttlet sample_src_offset = sample_nr * stride;

        // Offset the samples to the right side, so we can do left-shift-or to combine multiple loads.
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

[[nodiscard]] static i8x16 make_shuffle_shift(int num_samples) noexcept
{
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

[[nodiscard]] static float calculate_gain(int num_bytes, int num_integer_bits, int num_fraction_bits) noexcept
{
    // Find the maximum value of the fraction bits as a signed number.
    auto max_value = (1_uz << (num_fraction_bits - 1)) - 1;

    // Align left inside the int32_t.
    max_value <<= 32 - num_fraction_bits;

    // Add the integer bits on the left side.
    max_value >>= num_integer_bits;

    return 1.0f / narrow_cast<float>(max_value);
}

unpack_audio_samples::unpack_audio_samples(
    int num_bytes,
    int num_integer_bits,
    int num_fraction_bits,
    bool is_float,
    std::endian endian,
    int stride) noexcept :
    _num_bytes(num_bytes),
    _num_integer_bits(num_integer_bits),
    _num_fraction_bits(num_fraction_bits),
    _is_float(is_float),
    _endian(endian),
    _stride(stride)
{
    _num_samples_per_load = calculate_num_samples_per_load(num_bytes, stride);
    _shuffle_load = make_shuffle_load(num_bytes, endian, stride, _num_samples_per_load);
    _shuffle_shift = make_shuffle_shift(_num_samples_per_load);
    _gain = f32x4::broadcast(calculate_gain(num_bytes, num_integer_bits, num_fraction_bits));
}

[[nodiscard]] static int32_t load_sample(std::byte const *&src, int num_bytes, std::endian endian, size_t stride) noexcept
{
    tt_axiom(num_bytes > 0 and num_bytes <= 4);
    tt_axiom(stride >= num_bytes);

    int p_dir = 1;
    auto *p = src;
    if (endian == std::endian::little) {
        p_dir = -1;
        p += num_bytes - 1;
    }

    ttlet align_shift = 32 - num_bytes * 8;

    uint32_t r = 0;
    do {
        r <<= 8;
        r |= static_cast<uint32_t>(static_cast<uint8_t>(*p));
        p += p_dir;
    } while (--num_bytes);

    // Align the bits to the left to allow for sign extension.
    r <<= align_shift;

    src += stride;
    return static_cast<int32_t>(r);
}

[[nodiscard]] static i8x16 load_samples(std::byte const *&src, i8x16 shuffle_load, size_t load_stride) noexcept
{
    auto r = shuffle(i8x16::load(src), shuffle_load);
    src += load_stride;
    return r;
}

[[nodiscard]] static i32x4
load_samples(std::byte const *&src, i8x16 shuffle_load, i8x16 shuffle_shift, int num_loads, size_t load_stride) noexcept
{
    auto int_samples = i8x16::undefined();

    do {
        int_samples = shuffle(int_samples, shuffle_shift);
        int_samples |= load_samples(src, shuffle_load, load_stride);
    } while (--num_loads);

    return std::bit_cast<i32x4>(int_samples);
}

static void store_sample(float *&dst, float sample) noexcept
{
    *(dst++) = sample;
}

static void store_samples(float *&dst, f32x4 samples) noexcept
{
    samples.store(reinterpret_cast<std::byte *>(dst));
    dst += 4;
}

void unpack_audio_samples::operator()(std::byte const *tt_restrict src, float *tt_restrict dst, size_t num_samples) const noexcept
{
    // Calculate how many extra loads need to be done to complete a full 4 sample store.
    ttlet num_loads = (4 / _num_samples_per_load);
    ttlet load_stride = _stride * _num_samples_per_load;

    ttlet shuffle_load = _shuffle_load;
    ttlet shuffle_shift = _shuffle_shift;
    ttlet is_float = _is_float;
    ttlet gain = _gain;

    auto sample_nr = 0_uz;

    // Calculate a conservative number of samples that can be copied quickly
    // without overflowing the src buffer.
    ttlet num_samples_fast = floor(num_samples, 4_uz);
    for (; sample_nr != num_samples_fast; sample_nr += 4) {
        ttlet int_samples = load_samples(src, shuffle_load, shuffle_shift, num_loads, load_stride);

        auto float_samples = bit_cast<f32x4>(int_samples);
        if (!is_float) {
            float_samples = static_cast<f32x4>(int_samples);
            float_samples *= gain;
        }

        store_samples(dst, float_samples);
    }

    ttlet num_bytes = _num_bytes;
    ttlet endian = _endian;
    ttlet stride = _stride;
    for (; sample_nr != num_samples; ++sample_nr) {
        ttlet int_sample = load_sample(src, num_bytes, endian, stride);

        ttlet float_sample = is_float ? std::bit_cast<float>(int_sample) : static_cast<float>(int_sample) * gain[0];

        store_sample(dst, float_sample);
    }
}

} // namespace tt
