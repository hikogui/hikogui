// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "system_status.hpp"

#include <thread>

namespace tt {

void statistics_deinit() noexcept;

void statistics_init() noexcept;

inline bool statistics_start()
{
    return system_status_start_subsystem(system_status_type::statistics, statistics_init, statistics_deinit);
}

} // namespace tt
