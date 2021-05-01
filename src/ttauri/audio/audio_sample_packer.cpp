// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_sample_packer.hpp"
#include "../required.hpp"
#include "../cast.hpp"
#include "../memory.hpp"
#include "../endian.hpp"
#include "../geometry/numeric_array.hpp"
#include <bit>
#include <cstdint>
#include <tuple>

namespace tt {


static void store_sample(std::byte *&dst, int32_t x, int num_bytes, int direction, int start_byte, int align_shift) noexcept
{
    x >>= align_shift;

    auto p = dst + start_byte;
    do {
        *p = static_cast<std::byte>(x);
        p += direction;
        x >>= 8;
    } while (--num_bytes);
}

static void store_samples(std::byte *&dst, i32x4 x, int num_bytes, int direction, int start_byte, int align_shift) noexcept
{
    store_sample(dst, get<0>(x), num_bytes, direction, start_byte, align_shift);
    store_sample(dst, get<1>(x), num_bytes, direction, start_byte, align_shift);
    store_sample(dst, get<2>(x), num_bytes, direction, start_byte, align_shift);
    store_sample(dst, get<3>(x), num_bytes, direction, start_byte, align_shift);
}

[[nodiscard]] static float load_sample(float const *&src) noexcept
{
    return *(src++);
}

[[nodiscard]] static f32x4 load_samples(float const *&src) noexcept
{
    ttlet r = f32x4::load(src);
    src += 4;
    return r;
}


audio_sample_packer::audio_sample_packer(audio_sample_format format) noexcept : _format(format)
{
    _gain = f32x4::broadcast(format.pack_gain());

    _direction = format.endian == std::endian::little ? 1 : -1;
    _start_byte = format.endian == std::endian::little ? 0 : format.num_bytes - 1;
    _align_shift = 32 - format.num_bytes * 8;
}

[[nodiscard]] size_t audio_sample_packer::calculate_num_fast_samples(size_t num_samples) const noexcept
{
    return floor(num_samples, 4);
}

void audio_sample_packer::operator()(float const *tt_restrict src, std::byte *tt_restrict src, size_t num_samples)
    const noexcept
{
    tt_axiom(src != nullptr);
    tt_axiom(dst != nullptr);
    tt_axiom(_format.is_valid());

    // Calculate a conservative number of samples that can be copied quickly
    // without overflowing the src buffer.
    ttlet dst_end = dst + num_samples;
    ttlet dst_fast_end = dst + calculate_num_fast_samples(num_samples);

    if (_format.is_float) {
        while (dst != dst_fast_end) {
            ttlet float_samples = load_samples(src);
            ttlet int_samples = bit_cast<i32x4>(float_samples);
            store_samples(dst, int_samples);
        }
        while (dst != dst_end) {
            ttlet float_sample = load_sample(src);
            ttlet int_sample = std::bit_cast<int32_t>(float_sample);
            store_sample(dst, int_sample, _format.num_bytes,  _direction, _start_byte, _align_shift);
        }

    } else {
        ttlet gain = _gain;
        while (dst != dst_fast_end) {
            ttlet float_samples = load_samples(src);
            ttlet int_samples = static_cast<i32x4>(float_samples) * gain;
            store_samples(dst, int_samples);
        }
        while (dst != dst_end) {
            ttlet float_sample = load_sample(src);
            ttlet int_sample = static_cast<int32_t>(float_sample) * get<0>(gain);
            store_sample(dst, int_sample, _format.num_bytes, _direction, _start_byte, _align_shift);
        }
    }
}

} // namespace tt
