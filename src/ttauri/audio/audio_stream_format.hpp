// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_sample_format.hpp"
#include "speaker_mapping.hpp"

namespace tt {

/** The format of a stream of audio.
 */
struct audio_stream_format {
    audio_sample_format sample_format;

    int num_channels;

    speaker_mapping speaker_mapping;    
};

}

