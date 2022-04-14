// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "chrono.hpp"
#include "loop.hpp"
#include <chrono>
#include <coroutine>

namespace hi::inline v1 {

class awaitable_timer {
public:
    awaitable_timer(utc_nanoseconds deadline) noexcept : _deadline(deadline) {}

    awaitable_timer(std::chrono::nanoseconds period) noexcept : awaitable_timer(std::chrono::utc_clock::now() + period) {}

    [[nodiscard]] bool await_ready() const noexcept
    {
        return std::chrono::utc_clock::now() > _deadline;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept
    {
        _token = loop::local().delay_function(_deadline, [handle = std::move(handle)]() {
            handle.resume();
        });
    }

    void await_resume() const noexcept {}

private:
    utc_nanoseconds _deadline;
    loop::timer_token_type _token;
};

} // namespace hi::inline v1