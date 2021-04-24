// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../architecture.hpp"
#include "../geometry/numeric_array.hpp"
#include "audio_sample_format.hpp"
#include <cstddef>

namespace tt {

struct unpack_audio_samples_context {
    /** The amount of gain to apply to normalize samples between -1.0 and 1.0.
     */
    f32x4 gain;
};

/** Unpack and de-interleave samples from an audio channel.
 *
 * The destination floating point array must be a multiple of 4 floats, this function
 * may write 1 to 3 samples beyond the given count.
 *
 * @param src A pointer to the first sample of the channel.
 * @param src_format The format of the samples.
 * @param [out]dst A pointer to an flat array of floating point samples.
 * @param count The number of samples to convert.
 * @param [in,out]context The context from the previous iteration on the same channel.
 */
void unpack_audio_samples(
    std::byte const * tt_restrict src,
    audio_sample_format src_format,
    float *tt_restrict dst,
    size_t count,
    unpack_audio_samples_context &context);

} // namespace tt