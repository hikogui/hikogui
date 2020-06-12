// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/hires_utc_clock.hpp"
#include "TTauri/Foundation/audio_counter_clock.hpp"
#include "TTauri/Foundation/sync_clock.hpp"

namespace tt {

using audio_utc_clock = sync_clock<hires_utc_clock,audio_counter_clock>;

}
