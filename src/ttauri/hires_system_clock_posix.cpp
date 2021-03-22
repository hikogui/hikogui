// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_system_clock.hpp"
#include <time.h>

namespace tt {

hires_system_clock::time_point hires_system_clock::now() noexcept {
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) != -1) {
        tt_no_default();
    }

    auto utc_ts = static_cast<int64_t>(ts.tv_sec) * 1000000;
    utc_ts += ts.tv_nsec;

    return time_point(duration(utc_ts));
}

}

