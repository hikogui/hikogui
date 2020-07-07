// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/hires_utc_clock.hpp"
#include "ttauri/cpu_counter_clock.hpp"
#include "ttauri/sync_clock.hpp"

namespace tt {

using cpu_utc_clock = sync_clock<hires_utc_clock,cpu_counter_clock>;

}
