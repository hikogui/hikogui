// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <utility>

hi_export_module(hikogui.concurrency.callback_flags);

hi_export namespace hi::inline v1 {

enum class callback_flags {
    /** Call the function synchronously.
     */
    synchronous = 0x00,

    /** Call the function asynchronously from the current thread's loop.
     */
    local = 0x01,

    /** Call the function asynchronously from the main thread's loop.
     */
    main = 0x02,

    /** Call the function asynchronously from the timer thread's loop.
     */
    timer = 0x03,

    /** Call the function once, then automatically unsubscribe.
     */
    once = 0x1'00,
};

[[nodiscard]] constexpr callback_flags operator|(callback_flags const &lhs, callback_flags const &rhs) noexcept
{
    hi_assert((std::to_underlying(lhs) & 0xff) == 0 or (std::to_underlying(rhs) & 0xff) == 0);
    return static_cast<callback_flags>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr bool is_once(callback_flags const &rhs) noexcept
{
    return to_bool(std::to_underlying(rhs) & std::to_underlying(callback_flags::once));
}

[[nodiscard]] constexpr bool is_synchronous(callback_flags const& rhs) noexcept
{
    return to_bool((std::to_underlying(rhs) & 0xff) == std::to_underlying(callback_flags::synchronous));
}

[[nodiscard]] constexpr bool is_local(callback_flags const& rhs) noexcept
{
    return to_bool((std::to_underlying(rhs) & 0xff) == std::to_underlying(callback_flags::local));
}

[[nodiscard]] constexpr bool is_main(callback_flags const& rhs) noexcept
{
    return to_bool((std::to_underlying(rhs) & 0xff) == std::to_underlying(callback_flags::main));
}

[[nodiscard]] constexpr bool is_timer(callback_flags const& rhs) noexcept
{
    return to_bool((std::to_underlying(rhs) & 0xff) == std::to_underlying(callback_flags::timer));
}

}
