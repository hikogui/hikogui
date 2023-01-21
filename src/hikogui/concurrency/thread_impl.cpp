// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "thread.hpp"
#include "unfair_mutex.hpp"
#include "../utility/module.hpp"
#include <bit>
#include <unordered_map>
#include <string>
#include <mutex>
#include <format>

namespace hi::inline v1 {
namespace detail {

std::unordered_map<thread_id, std::string> thread_names = {};
unfair_mutex thread_names_mutex = {};

} // namespace detail

[[nodiscard]] std::string get_thread_name(thread_id id) noexcept
{
    hilet lock = std::scoped_lock(detail::thread_names_mutex);
    hilet it = detail::thread_names.find(id);
    if (it != detail::thread_names.end()) {
        return it->second;
    } else {
        return std::format("{}", id);
    }
}

std::vector<bool> set_thread_affinity(std::size_t cpu_id)
{
    auto new_mask = std::vector<bool>{};
    new_mask.resize(cpu_id + 1);
    new_mask[cpu_id] = true;
    return set_thread_affinity_mask(new_mask);
}

std::size_t advance_thread_affinity(std::size_t &cpu) noexcept
{
    auto available_cpus = process_affinity_mask();
    hi_assert_bounds(cpu, available_cpus);

    ssize_t selected_cpu = -1;
    do {
        if (available_cpus[cpu]) {
            try {
                set_thread_affinity(cpu);
                selected_cpu = narrow_cast<ssize_t>(cpu);
            } catch (os_error const &) {
            }
        }

        // Advance to the next available cpu.
        // We do this, so that the caller of this function can detect a wrap around.
        do {
            if (++cpu >= available_cpus.size()) {
                cpu = 0;
            }
        } while (!available_cpus[cpu]);
    } while (selected_cpu < 0);

    return narrow_cast<std::size_t>(selected_cpu);
}

} // namespace hi::inline v1