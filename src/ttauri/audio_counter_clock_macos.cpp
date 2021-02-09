// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_counter_clock.hpp"
#include <mach/mach_time.h>

namespace tt {

using namespace std::chrono_literals;

audio_counter_clock::time_point audio_counter_clock::from_audio_api(uint64_t value) noexcept
{
    // The timepoint is based on a value that is near 1 ns.
    return time_point(value * 1ns);
}

audio_counter_clock::time_point audio_counter_clock::now() noexcept
{
    auto counter = mach_absolute_time();

    return time_point(duration(counter));
}

}
