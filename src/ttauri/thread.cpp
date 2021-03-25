// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "thread.hpp"
#include "logger.hpp"
#include <bit>

namespace tt {

uint64_t set_thread_affinity(size_t processor_index) noexcept
{
    auto available_mask = process_affinity_mask();
    auto new_mask = uint64_t{1} << processor_index;
    if ((available_mask & new_mask) == 0) {
        tt_log_error("Current process {:x} does not match process_afinity_mask {:x}", new_mask, available_mask);
        new_mask = std::bit_floor(available_mask);
    }
    return set_thread_affinity_mask(new_mask);
}

}