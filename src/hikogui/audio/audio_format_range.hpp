// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pcm_format.hpp"

namespace hi::inline v1 {

class audio_format_range {
    uint32_t min_sample_rate;
    uint32_t max_sample_rate;
    uint16_t min_channels;
    uint16_t max_channels;
    pcm_format format;
};

}
