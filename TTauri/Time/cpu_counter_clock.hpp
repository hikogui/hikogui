// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <chrono>

namespace TTauri::Time {

struct cpu_counter_clock {
    using rep = uint64_t;
    // Technically not nano-seconds, this just a counter.
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using timepoint = std::chrono::time_point<cpu_counter_clock>;
    static const bool is_steady = true;

    static timepoint now();
};

}
