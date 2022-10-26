// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_sample_format.hpp"
#include "../utility.hpp"
#include "../architecture.hpp"
#include "../rapid/numeric_array.hpp"
#include <cstddef>
#include <bit>

namespace hi::inline v1 {

class audio_sample_unpacker {
public:
    /** Audio sample unpacker
     * One instance of this class can be used to unpack multiple buffers either
     * from one audio-proc to the next, or for each channel in a group of
     * interleaved channels.
     *
     * @param format The sample format.
     * @param stride The distance to the next sample.
     */
    audio_sample_unpacker(audio_sample_format format, std::size_t stride) noexcept;

    /** Unpack samples.
     *
     * @param src A pointer to a byte array containing samples.
     * @param dst A pointer to a array of floating point samples of a single channel.
     * @param num_samples Number of samples.
     */
    void operator()(std::byte const *hi_restrict src, float *hi_restrict dst, std::size_t num_samples) const noexcept;

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

    [[nodiscard]] std::size_t calculate_num_fast_samples(std::size_t num_samples) const noexcept;
};

} // namespace hi::inline v1
