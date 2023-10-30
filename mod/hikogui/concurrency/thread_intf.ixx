// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file concurrency/thread.hpp Functions and types for accessing operating system threads.
 * @ingroup concurrency
 */

module;
#include "../macros.hpp"

#include <thread>
#include <string>
#include <string_view>
#include <format>
#include <functional>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <bit>

export module hikogui_concurrency_thread : intf;
import hikogui_concurrency_unfair_mutex;
import hikogui_utility;

export namespace hi { inline namespace v1 {
namespace detail {

std::unordered_map<thread_id, std::string> thread_names = {};
unfair_mutex thread_names_mutex = {};

} // namespace detail

/** Get the current thread id.
 * Get the current thread id quickly.
 *
 * @ingroup concurrency
 */
[[nodiscard]] thread_id current_thread_id() noexcept;

/** Set the name of the current thread.
 * This function will set the name of the thread so that it is available
 * by the operating system and debugger.
 *
 * Every thread should call this function exactly once.
 *
 * @ingroup concurrency
 */
void set_thread_name(std::string_view name) noexcept;

/** Get the thread name of a thread id.
 *
 * This function is designed to be reasonably fast, so that it can be used
 * in the logger thread.
 *
 * @ingroup concurrency
 * @note Can only read names that where set using set_thread_name().
 * @param id The thread id.
 * @return Name of the thread set with `set_hread_name()`. Or the numeric thread-id as string.
 */
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

/** Get the current process CPU affinity mask.
 *
 * @ingroup concurrency
 * @return A bit mask on which CPUs the process is allowed to run on.
 *         Or zero on failure.
 */
[[nodiscard]] std::vector<bool> process_affinity_mask();

/** Set the current thread CPU affinity mask.
 *
 * The given mask must be a strict subset of the mask returned from
 * process_affinity_mask().
 *
 * @ingroup concurrency
 * @param mask A bit mask on which CPUs the thread should run on.
 * @throw std::os_error When unable to set the thread affinity to the given index
 * @return The previous bit mask. Or zero on failure.
 */
std::vector<bool> set_thread_affinity_mask(std::vector<bool> const& mask);

/** Set the current thread CPU affinity to a single CPU.
 *
 * The given processor index must be a part of the mask returned from
 * process_affinity_mask().
 *
 * @ingroup concurrency
 * @param cpu_id The index of the CPU the thread should run on.
 * @return The previous bit mask. Or zero on failure.
 * @throw std::os_error When unable to set the thread affinity to the given index
 */
std::vector<bool> set_thread_affinity(std::size_t cpu_id)
{
    auto new_mask = std::vector<bool>{};
    new_mask.resize(cpu_id + 1);
    new_mask[cpu_id] = true;
    return set_thread_affinity_mask(new_mask);
}

/** Advance thread affinity to the next CPU.
 * It is possible to detect when `advance_thread_affinity()` is at the last cpu;
 * in that case the cpu parameter is less than or equal to the return value.
 *
 * @ingroup concurrency
 * @param[inout] cpu On input The cpu to start a search in the available-cpu list.
 *                    On output the cpu next on the available-cpu list.
 *
 * @return The cpu that was selected to run on.
 */
[[nodiscard]] std::size_t advance_thread_affinity(std::size_t &cpu) noexcept
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

/** Get the current CPU id.
 *
 * @ingroup concurrency
 * @return The current CPU id.
 */
[[nodiscard]] std::size_t current_cpu_id() noexcept;

}} // namespace hi::v1
