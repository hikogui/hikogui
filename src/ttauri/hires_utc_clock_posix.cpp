// Copyright 2019 Pokitec
// All rights reserved.

#include "hires_utc_clock.hpp"
#include <time.h>

namespace tt {

hires_utc_clock::time_point hires_utc_clock::now() noexcept {
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) != -1) {
        tt_no_default;
    }

    auto utc_ts = static_cast<int64_t>(ts.tv_sec) * 1000000;
    utc_ts += ts.tv_nsec;

    return time_point(duration(utc_ts));
}

}

