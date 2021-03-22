// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "subsystem.hpp"
#include <atomic>

namespace tt {

void statistics_deinit() noexcept;

bool statistics_init() noexcept;

inline std::atomic<bool> statistics_running = false;

inline bool statistics_start()
{
    return start_subsystem(statistics_running, false, statistics_init, statistics_deinit);
}

} // namespace tt
