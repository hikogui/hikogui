// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_sample_format.hpp"
#include "../required.hpp"
#include "../architecture.hpp"
#include "../geometry/numeric_array.hpp"
#include <cstddef>
#include <bit>

namespace tt {

class audio_sample_unpacker {
public:
    /** Audio sample unpacker
     * One instance of this class can be used to unpack multiple buffers either
     * from one audio-proc to the next, or for each channel in a group of
     * interleaved channels.
     *
     * @param format The sample format.
     */
    audio_sample_unpacker(audio_sample_format format) noexcept;

    /** Unpack samples.
     *
     * @param src A pointer to a byte array containing samples.
     * @param dst A pointer to a array of floating point samples of a single channel.
     * @param num_samples Number of samples.
     */
    void operator()(std::byte const *tt_restrict src, float *tt_restrict dst, size_t num_samples) const noexcept;

private:
    audio_sample_format _format;

    i8x16 _shuffle_load;
    i8x16 _shuffle_shift;
};

} // namespace tt
