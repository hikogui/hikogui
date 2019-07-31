// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <chrono>

namespace TTauri::Time {

/*! Timestamp
 */
struct hires_utc_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using timepoint = std::chrono::time_point<hires_utc_clock>;
    static const bool is_steady = false;

	static timepoint now();
};

}

