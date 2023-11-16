// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file debugger.ixx Utilities to interact with the debugger this application runs under.
 */

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.utils.debugger : utils);

hi_export namespace hi { inline namespace v1 {
namespace detail {
    /** Message to show when the application is terminated because of a debug_abort.
 */
hi_inline std::atomic<char const *> debug_message = nullptr;

}

hi_inline void set_debug_message(char const *str) noexcept
{
    detail::debug_message.store(str, std::memory_order::relaxed);
}

[[nodiscard]] hi_inline bool has_debug_message() noexcept
{
    return detail::debug_message.load(std::memory_order::relaxed) != nullptr;
}

[[nodiscard]] hi_inline char const *get_debug_message() noexcept
{
    return detail::debug_message.exchange(nullptr, std::memory_order::relaxed);
}

}}