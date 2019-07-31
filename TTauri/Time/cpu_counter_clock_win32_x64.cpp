// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "cpu_counter_clock.hpp"
#ifdef _WIN32
#include <intrin.h>
#endif

namespace TTauri::Time {

static timepoint cpu_counter_clock::now() {
    return timepoint(duration(__rdtsc()));
}

}
