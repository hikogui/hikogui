// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include <chrono>
#if COMPILER == CC_MSVC
#include <intrin.h>
#elif COMPILER == CC_CLANG
#include <x86intrin.h>
#elif COMPILER == CC_GCC
#include <x86intrin.h>
#endif

namespace TTauri {

struct cpu_counter_clock {
    using rep = uint64_t;
    // Technically not nano-seconds, this just a counter.
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<cpu_counter_clock>;
    static const bool is_steady = true;

    static cpu_counter_clock::time_point now() noexcept {
        return time_point(duration(__rdtsc()));
    }
};

}
