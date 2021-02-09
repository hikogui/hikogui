// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "hires_utc_clock.hpp"
#include "cpu_counter_clock.hpp"
#include "sync_clock.hpp"

namespace tt {

using cpu_utc_clock = sync_clock<hires_utc_clock,cpu_counter_clock>;

}
