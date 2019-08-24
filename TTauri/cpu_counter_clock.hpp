// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "os_detect.hpp"
#include <chrono>
#if OPERATING_SYSTEM == OS_WINDOWS
#include <intrin.h>
#endif

namespace TTauri {

struct cpu_counter_clock {
    using rep = uint64_t;
    // Technically not nano-seconds, this just a counter.
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<cpu_counter_clock>;
    static const bool is_steady = true;

    static cpu_counter_clock::time_point cpu_counter_clock::now() {
#if OPERATING_SYSTEM == OS_WINDOWS
        return time_point(duration(__rdtsc()));
#else
#error "Not implemented"
#endif
    }
};

}
