// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "hires_tai_clock.hpp"
#include "hires_utc_clock.hpp"
#include "leapsecond_db.hpp"

namespace TTauri::Time {

/*! Get a timestamp based on a high resolution system clock.
 */
static timepoint hires_tai_clock::now() {
    return get_singleton<leapsecond_db>().UTC_to_TAI(hires_utc_clock::now());
}

};

}
