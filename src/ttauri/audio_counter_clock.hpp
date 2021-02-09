// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "os_detect.hpp"
#include <chrono>
#if TT_COMPILER == TT_CC_MSVC
#include <intrin.h>
#elif TT_COMPILER == TT_CC_CLANG
#include <x86intrin.h>
#elif TT_COMPILER == TT_CC_GCC
#include <x86intrin.h>
#endif

namespace tt {

struct audio_counter_clock {
    using rep = uint64_t;
    // Technically not nano-seconds, this just a counter.
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<audio_counter_clock>;
    static const bool is_steady = true;

    static audio_counter_clock::time_point from_audio_api(uint64_t value) noexcept;

    static audio_counter_clock::time_point now() noexcept;
};

}
