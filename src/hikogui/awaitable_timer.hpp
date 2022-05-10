// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "awaitable.hpp"
#include "chrono.hpp"
#include <chrono>
#include <coroutine>
#include <functional>

namespace hi::inline v1 {

class awaitable_timer {
public:
    template<typename Duration>
    awaitable_timer(std::chrono::time_point<std::chrono::utc_clock, Duration> deadline) noexcept : _deadline(deadline) {}

    template<typename Rep, typename Period>
    awaitable_timer(std::chrono::duration<Rep,Period> period) noexcept : awaitable_timer(std::chrono::utc_clock::now() + period) {}

    [[nodiscard]] bool await_ready() const noexcept
    {
        return std::chrono::utc_clock::now() > _deadline;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept;

    void await_resume() const noexcept {}

private:
    utc_nanoseconds _deadline;
    std::shared_ptr<std::function<void()>> _token;
};

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
template<typename Rep, typename Period>
struct awaitable_cast<std::chrono::duration<Rep, Period>> {
    using type = awaitable_timer;

    [[nodiscard]] type operator()(auto&& rhs) const noexcept
    {
        return awaitable_timer{hi_forward(rhs)};
    }
};

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
template<typename Duration>
struct awaitable_cast<std::chrono::time_point<std::chrono::utc_clock, Duration>> {
    using type = awaitable_timer;

    [[nodiscard]] type operator()(auto&& rhs) const noexcept
    {
        return awaitable_timer{hi_forward(rhs)};
    }
};

} // namespace hi::inline v1