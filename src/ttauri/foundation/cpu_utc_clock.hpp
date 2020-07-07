// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/hires_utc_clock.hpp"
#include "ttauri/foundation/cpu_counter_clock.hpp"
#include "ttauri/foundation/sync_clock.hpp"

namespace tt {

using cpu_utc_clock = sync_clock<hires_utc_clock,cpu_counter_clock>;

}
