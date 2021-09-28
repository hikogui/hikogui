// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "thread.hpp"
#include "log.hpp"
#include "exception.hpp"
#include "GUI/gui_system.hpp"
#include <bit>

namespace tt {

std::vector<bool> set_thread_affinity(size_t cpu_id)
{
    auto new_mask = std::vector<bool>{};
    new_mask.resize(cpu_id + 1);
    new_mask[cpu_id] = true;
    return set_thread_affinity_mask(new_mask);
}

size_t advance_thread_affinity(size_t &cpu) noexcept
{
    auto available_cpus = process_affinity_mask();
    tt_axiom(cpu < available_cpus.size());

    ssize_t selected_cpu = -1;
    do {
        if (available_cpus[cpu]) {
            try {
                set_thread_affinity(cpu);
                selected_cpu = narrow_cast<ssize_t>(cpu);
            } catch (os_error const &) {}
        }

        // Advance to the next available cpu.
        // We do this, so that the caller of this function can detect a wrap around.
        do {
            if (++cpu >= available_cpus.size()) {
                cpu = 0;
            }
        } while (!available_cpus[cpu]);
    } while (selected_cpu < 0);

    return narrow_cast<size_t>(selected_cpu);
}

}