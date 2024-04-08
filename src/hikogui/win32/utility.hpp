// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "../macros.hpp"
#include <utility>
#include <cstdint>
#include <Windows.h>

hi_export_module(hikogui.win32 : utility);

hi_export namespace hi { inline namespace v1 {

/** Convert a HANDLE to a 32-bit unsigned integer.
 *
 * Although a `HANDLE` is a typedef of a `void *`, in reality it is an
 * 32 bit unsigned integer.
 *
 * This function is used to pass a HANDLE of an Event Object to be passed
 * on the command-line to vsjitdebugger.exe.
 */
[[nodiscard]] inline uint32_t win32_HANDLE_to_int(HANDLE handle) noexcept
{
    auto i = std::bit_cast<uintptr_t>(handle);
    if (std::cmp_greater(i, std::numeric_limits<uint32_t>::max())) {
        std::terminate();
    }
    return static_cast<uint32_t>(i);
}

[[nodiscard]] inline HANDLE win32_int_to_HANDLE(uint32_t i) noexcept
{
    return std::bit_cast<HANDLE>(static_cast<uintptr_t>(i));
}

}} // namespace hi::v1
