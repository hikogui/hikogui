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

audio_sample_unpacker::audio_sample_unpacker(audio_sample_format format) noexcept : _format(format)
{
    _shuffle_load = make_shuffle_load(format);
    _shuffle_shift = make_shuffle_shift(format);

    _multiplier = f32x4::broadcast(format.unpack_multiplier());
    _num_samples_per_load = format.samples_per_load();
    _num_loads_per_store = format.loads_per_store();
    _load_stride = format.load_stride();

    _direction = format.endian == std::endian::little ? -1 : 1;
    _start_byte = format.endian == std::endian::little ? format.num_bytes - 1 : 0;
    _align_shift = 32 - format.num_bytes * 8;
}

[[nodiscard]] size_t audio_sample_unpacker::calculate_num_fast_samples(size_t num_samples) const noexcept
{
    tt_axiom(_format.is_valid());
    tt_axiom(_num_samples_per_load == 1 || _num_samples_per_load == 2 || _num_samples_per_load == 4);
    tt_axiom(_num_loads_per_store == 1 || _num_loads_per_store == 2 || _num_loads_per_store == 4);
    tt_axiom(_load_stride > 0);

    ttlet src_buffer_size = (num_samples - 1) * _format.stride + _format.num_bytes;
    if (src_buffer_size < 16) {
        return 0;
    }

    auto num_loads = (src_buffer_size - 16) / _load_stride + 1;
    auto num_stores = num_loads / _num_loads_per_store;
    return num_stores * 4;
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
    ttlet dst_fast_end = dst + calculate_num_fast_samples(num_samples);

    auto square_peak = f32x4{};
    auto square_sum = f32x4{};

    if (_format.is_float) {
        while (dst != dst_fast_end) {
            ttlet int_samples = load_samples(src, _shuffle_load, _shuffle_shift, _num_loads_per_store, _load_stride);
            ttlet float_samples = bit_cast<f32x4>(int_samples);

            ttlet square_samples = float_samples * float_samples;
            square_sum += square_samples
            square_peak = max(peak, square_samples);

            store_samples(dst, float_samples);
        }
        while (dst != dst_end) {
            ttlet int_sample = load_sample(src, _format.stride, _format.num_bytes, _direction, _start_byte, _align_shift);
            ttlet float_sample = std::bit_cast<float>(int_sample);

            ttlet square_sample = float_sample * float_sample;
            square_sum.x() += square_sample;
            square_peak.x() = max(square_peak.x(), square_sample);

            store_sample(dst, float_sample);
        }

    } else {
        ttlet multiplier = _multiplier;

        while (dst != dst_fast_end) {
            ttlet int_samples = load_samples(src, _shuffle_load, _shuffle_shift, _num_loads_per_store, _load_stride);
            ttlet float_samples = static_cast<f32x4>(int_samples) * multiplier;

            ttlet square_samples = float_samples * float_samples;
            square_sum += square_samples
            square_peak = max(peak, square_samples);

            store_samples(dst, float_samples);
        }
        while (dst != dst_end) {
            ttlet int_sample = load_sample(src, _format.stride, _format.num_bytes, _direction, _start_byte, _align_shift);
            ttlet float_sample = static_cast<float>(int_sample) * get<0>(multiplier);

            ttlet square_sample = float_sample * float_sample;
            square_sum.x() += square_sample
            square_peak.x() = std::max(square_peak.x(), square_sample);

            store_sample(dst, float_sample);
        }
    }

    square_sum = hadd(square_sum, square_sum);
    square_sum = hadd(square_sum, square_sum);
    square_peak = hmax(square_peak square_peak);
    square_peak = hmax(square_peak square_peak);

    return f32x4{square_peak, square_sum, static_cast<float>(num_samples), 0.0f};
}

} // namespace tt
