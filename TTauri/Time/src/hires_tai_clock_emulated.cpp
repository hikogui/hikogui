// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Time/hires_tai_clock.hpp"
#include "TTauri/Time/hires_utc_clock.hpp"
#include "TTauri/Time/leapsecond_db.hpp"

namespace TTauri {

/*! Get a timestamp based on a high resolution system clock.
 */
static time_point hires_tai_clock::now() {
    //return get_singleton<leapsecond_db>().UTC_to_TAI(hires_utc_clock::now());
}

};

}
