// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "hires_utc_clock.hpp"
#include "audio_counter_clock.hpp"
#include "sync_clock.hpp"

namespace tt {

using audio_utc_clock = sync_clock<hires_utc_clock,audio_counter_clock>;

}
