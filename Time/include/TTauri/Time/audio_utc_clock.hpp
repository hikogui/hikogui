// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Time/hires_utc_clock.hpp"
#include "TTauri/Time/audio_counter_clock.hpp"
#include "TTauri/Time/sync_clock.hpp"

namespace TTauri {

using audio_utc_clock = sync_clock<hires_utc_clock,audio_counter_clock>;

}
