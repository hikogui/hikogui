// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "os_detect.hpp"
#include "hires_utc_clock.hpp"
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include <intrin.h>
#endif
#include <thread>
#include <string_view>
#include <functional>
#include <atomic>
#include <chrono>

namespace tt {

/** Set the name of the current thread.
 * This function will set the name of the thread so that it is available
 * by the operating system and debugger.
 *
 * Every thread should call this function exactly once.
 */
void set_thread_name(std::string_view name);

bool is_main_thread();

void run_from_main_loop(std::function<void()> f);

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
using thread_id = uint32_t;
#else
using thread_id = uint64_t;
/** A dummy variable to use as an address inside the TLS, used as a thread_id.
 */
inline thread_local thread_id current_thread_id_dummy = 0;
#endif

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
 * On systems with more than 64 processors, this returns a mask
 * of logical processors within the processor-group.
 *
 * @return A bit mask on which processors the process is allowed to run on.
 *         Or zero on failure.
 */
[[nodiscard]] uint64_t process_affinity_mask() noexcept;

/** Set the current thread CPU affinity mask.
 * On systems with more than 64 processors, this sets a mask
 * of logical processors within the processor-group.
 *
 * The given mask must be a strict subset of the mask returned from
 * process_affinity_mask().
 *
 * @param mask A bit mask on which processors the thread should run on.
 * @return The previous bit mask. Or zero on failure.
 */
[[nodiscard]] uint64_t set_thread_affinity_mask(uint64_t mask) noexcept;



}

