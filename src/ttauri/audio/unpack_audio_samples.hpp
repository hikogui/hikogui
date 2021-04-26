// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../architecture.hpp"
#include "../geometry/numeric_array.hpp"
#include <cstddef>
#include <bit>

namespace tt {

class unpack_audio_samples {
public:
    unpack_audio_samples(
        int num_bytes,
        int num_integer_bits,
        int num_fraction_bits,
        bool is_float,
        std::endian endian,
        int stride) noexcept;

    void operator()(std::byte const *tt_restrict src, float *tt_restrict dst, size_t num_samples) const noexcept;

private:
    int _num_bytes;
    int _num_integer_bits;
    int _num_fraction_bits;
    bool _is_float;
    std::endian _endian;
    int _stride;

    int _num_samples_per_load;
    i8x16 _shuffle_load;
    i8x16 _shuffle_shift;
    f32x4 _gain;
};

} // namespace tt