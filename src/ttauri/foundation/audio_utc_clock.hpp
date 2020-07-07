// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/hires_utc_clock.hpp"
#include "ttauri/foundation/audio_counter_clock.hpp"
#include "ttauri/foundation/sync_clock.hpp"

namespace tt {

using audio_utc_clock = sync_clock<hires_utc_clock,audio_counter_clock>;

}
