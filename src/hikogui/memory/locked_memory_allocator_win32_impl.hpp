// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"

#include "locked_memory_allocator_intf.hpp"
#include "../telemetry/telemetry.hpp"
#include "../char_maps/char_maps.hpp" // XXX #616
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <format>
#include <type_traits>
#include <string_view>
#include <string>
#include <compare>
#include <exception>
#include <chrono> // XXX #620

hi_export_module(hikogui.memory.locked_memory_allocator : impl);

hi_export namespace hi::inline v1 {

[[nodiscard]] hi_inline std::byte *locked_memory_allocator_allocate(std::size_t n) noexcept
{
    auto p = VirtualAlloc(NULL, n, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (p == NULL) {
        hi_log_fatal("Could not allocate locked memory. '{}'", get_last_error_message());
    }

    auto handle = GetCurrentProcess();
    SIZE_T minimum_working_set_size;
    SIZE_T maximum_working_set_size;

    if (!GetProcessWorkingSetSize(handle, &minimum_working_set_size, &maximum_working_set_size)) {
        hi_log_warning("Could not get process working set size. '{}'", get_last_error_message());

    } else {
        minimum_working_set_size += ceil(n, SIZE_T{4096});
        maximum_working_set_size += ceil(n, SIZE_T{4096});
        if (!SetProcessWorkingSetSize(handle, minimum_working_set_size, maximum_working_set_size)) {
            hi_log_warning(
                "Could not set process working set size to {}:{}. '{}'",
                minimum_working_set_size,
                maximum_working_set_size,
                get_last_error_message());

        } else {
            if (!VirtualLock(p, n)) {
                hi_log_warning("Could not lock memory. '{}'", get_last_error_message());
            }
        }
    }
    return static_cast<std::byte *>(p);
}

hi_inline void locked_memory_allocator_deallocate(std::byte *p, std::size_t n) noexcept
{
    if (not VirtualUnlock(p, n)) {
        hi_log_warning("Could not unlock memory. '{}'", get_last_error_message());

    } else {
        auto handle = GetCurrentProcess();
        SIZE_T minimum_working_set_size;
        SIZE_T maximum_working_set_size;

        if (!GetProcessWorkingSetSize(handle, &minimum_working_set_size, &maximum_working_set_size)) {
            hi_log_warning("Could not get process working set size. '{}'", get_last_error_message());

        } else {
            minimum_working_set_size -= ceil(n, SIZE_T{4096});
            maximum_working_set_size -= ceil(n, SIZE_T{4096});
            if (!SetProcessWorkingSetSize(handle, minimum_working_set_size, maximum_working_set_size)) {
                hi_log_warning(
                    "Could not set process working set size to {}:{}. '{}'",
                    minimum_working_set_size,
                    maximum_working_set_size,
                    get_last_error_message());
            }
        }
    }

    if (not VirtualFree(p, 0, MEM_RELEASE)) {
        hi_log_fatal("Could not deallocate locked memory. '{}'", get_last_error_message());
    }
}

} // namespace hi::inline v1
