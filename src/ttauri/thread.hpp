// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "architecture.hpp"
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include <intrin.h>
#endif
#include <thread>
#include <string_view>
#include <functional>
#include <atomic>
#include <chrono>
#include <bit>

namespace tt {

/** Set the name of the current thread.
 * This function will set the name of the thread so that it is available
 * by the operating system and debugger.
 *
 * Every thread should call this function exactly once.
 */
void set_thread_name(std::string_view name);

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
constexpr size_t maximum_num_cpus = 64;
#elif TT_OPERATING_SYSTEM == TT_OS_LINUX || TT_OPERATING_SYSTEM == TT_OS_MACOS
constexpr size_t maximum_num_cpus = CPU_SETSIZE;
#endif

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
using thread_id = uint32_t;
#else
using thread_id = uint64_t;
/** A dummy variable to use as an address inside the TLS, used as a thread_id.
 */
inline thread_local thread_id current_thread_id_dummy = 0;
#endif

/** Get the current thread id.
 * Get the current thread id quickly.
 */
[[nodiscard]] inline thread_id current_thread_id() noexcept
{
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    // Thread IDs on Win32 are guaranteed to be not zero.
    constexpr uint64_t NT_TIB_CurrentThreadID = 0x48;
    return __readgsdword(NT_TIB_CurrentThreadID);
#else
    // Addresses can not be zero.
    return reinterpret_cast<uint64_t>(&current_thread_id_dummy);
#endif
}

/** Get the current process CPU affinity mask.
 *
 * @return A bit mask on which CPUs the process is allowed to run on.
 *         Or zero on failure.
 */
[[nodiscard]] std::vector<bool> process_affinity_mask() noexcept;

/** Set the current thread CPU affinity mask.
 *
 * The given mask must be a strict subset of the mask returned from
 * process_affinity_mask().
 *
 * @param mask A bit mask on which CPUs the thread should run on.
 * @throw std::os_error When unable to set the thread affinity to the given index
 * @return The previous bit mask. Or zero on failure.
 */
std::vector<bool> set_thread_affinity_mask(std::vector<bool> const &mask);

/** Set the current thread CPU affinity to a single CPU.
 *
 * The given processor index must be a part of the mask returned from
 * process_affinity_mask().
 *
 * @param processor_index The index of the CPU the thread should run on.
 * @return The previous bit mask. Or zero on failure.
 * @throw std::os_error When unable to set the thread affinity to the given index
 */
std::vector<bool> set_thread_affinity(size_t cpu_id);

/** Advance thread affinity to the next CPU.
 * It is possible to detect when `advance_thread_affinity()` is at the last cpu;
 * in that case the cpu parameter is less than or equal to the return value.
 * 
 * @param [inout] cpu On input The cpu to start a search in the available-cpu list.
 *                    On output the cpu next on the available-cpu list.
 *                                
 * @return The cpu that was selected to run on.
 */
size_t advance_thread_affinity(size_t &cpu) noexcept;

/** Get the current CPU id.
 *
 * @return The current CPU id.
 */
[[nodiscard]] size_t current_cpu_id() noexcept;


}

