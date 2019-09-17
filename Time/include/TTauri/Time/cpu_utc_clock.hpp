// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Time/hires_utc_clock.hpp"
#include "TTauri/Time/cpu_counter_clock.hpp"
#include "TTauri/Time/sync_clock.hpp"

namespace TTauri {

using cpu_utc_clock = sync_clock<hires_utc_clock,cpu_counter_clock>;

}
