// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "locked_memory_allocator.hpp"
#include "log.hpp"
#include "memory.hpp"
#include <Windows.h>

namespace tt::inline v1 {

[[nodiscard]] std::byte *locked_memory_allocator_allocate(size_t n) noexcept
{
    auto p = VirtualAlloc(NULL, n, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (p == NULL) {
        tt_log_fatal("Could not allocate locked memory. '{}'", get_last_error_message());
    }

    auto handle = GetCurrentProcess();
    SIZE_T minimum_working_set_size;
    SIZE_T maximum_working_set_size;

    if (!GetProcessWorkingSetSize(handle, &minimum_working_set_size, &maximum_working_set_size)) {
        tt_log_warning("Could not get process working set size. '{}'", get_last_error_message());

    } else {
        minimum_working_set_size += ceil(n, SIZE_T{4096});
        maximum_working_set_size += ceil(n, SIZE_T{4096});
        if (!SetProcessWorkingSetSize(handle, minimum_working_set_size, maximum_working_set_size)) {
            tt_log_warning(
                "Could not set process working set size to {}:{}. '{}'",
                minimum_working_set_size,
                maximum_working_set_size,
                get_last_error_message());

        } else {
            if (!VirtualLock(p, n)) {
                tt_log_warning("Could not lock memory. '{}'", get_last_error_message());
            }
        }
    }
    return reinterpret_cast<std::byte *>(p);
}

void locked_memory_allocator_deallocate(std::byte *p, size_t n) noexcept
{
    if (!VirtualUnlock(reinterpret_cast<LPVOID>(p), n)) {
        tt_log_warning("Could not unlock memory. '{}'", get_last_error_message());

    } else {
        auto handle = GetCurrentProcess();
        SIZE_T minimum_working_set_size;
        SIZE_T maximum_working_set_size;

        if (!GetProcessWorkingSetSize(handle, &minimum_working_set_size, &maximum_working_set_size)) {
            tt_log_warning("Could not get process working set size. '{}'", get_last_error_message());

        } else {
            minimum_working_set_size -= ceil(n, SIZE_T{4096});
            maximum_working_set_size -= ceil(n, SIZE_T{4096});
            if (!SetProcessWorkingSetSize(handle, minimum_working_set_size, maximum_working_set_size)) {
                tt_log_warning(
                    "Could not set process working set size to {}:{}. '{}'",
                    minimum_working_set_size,
                    maximum_working_set_size,
                    get_last_error_message());
            }
        }
    }

    if (!VirtualFree(reinterpret_cast<LPVOID>(p), 0, MEM_RELEASE)) {
        tt_log_fatal("Could not deallocate locked memory. '{}'", get_last_error_message());
    }
}

} // namespace tt
