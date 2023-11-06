// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <utility>
#include <coroutine>
#include <chrono>

export module hikogui_dispatch : awaitable_timer_impl;
import : awaitable_timer_intf;
import : loop_intf;

export namespace hi::inline v1 {

void awaitable_timer::await_suspend(std::coroutine_handle<> handle) noexcept
{
    _callback = loop::local().delay_function(_deadline, [handle = std::move(handle)]() {
        handle.resume();
    });
}

}
