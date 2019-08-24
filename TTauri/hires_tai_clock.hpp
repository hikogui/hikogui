// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri {

/*! Create a tai clock from a utc-clock.
 */
struct hires_tai_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<hires_utc_clock>;
    static const bool is_steady = false;

	static time_point now();
};

}
