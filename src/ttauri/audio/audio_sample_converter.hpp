// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>
#include "audio_sample_format.hpp"

namespace tt {

struct flatten_audio_channel_context {
};

struct unflatten_audio_channel_context {
};



/** interleave and pack samples from an audio channel.
 *
 * The source floating point array must be a multiple of 4 floats, this function
 * may read 1 to 3 samples beyond the given count.
 *
 * @param src A pointer to an flat array of floating point samples.
 * @param [out]dst A pointer to the first sample of the channel.
 * @param dst_format The format of the samples.
 * @param count The number of samples to convert.
 * @param [in,out]context The context from the previous iteration on the same channel.
 */
void unflatten_audio_channel(
    float const *src,
    std::byte *dst,
    audio_sample_format dst_format,
    size_t count,
    unflatten_audio_channel_context &context);

} // namespace tt
