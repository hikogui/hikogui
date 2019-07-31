// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "hires_utc_clock.hpp"

namespace TTauri::Time {

static timepoint hires_utc_clock::now() {
    struct timespec ts;

    // This should never return an error, but it needs to be fast too.
    clock_gettime(CLOCK_REALTIME, &ts);

    auto utc_ts = static_cast<int64_t>(ts.tv_sec) * 1000000;
    utc_ts += ts.tv_nsec;

    return timepoint(duration(utc_ts));
}

}

