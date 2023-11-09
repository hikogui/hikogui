// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <chrono>
#include <coroutine>
#include <functional>

export module hikogui_dispatch : awaitable_timer_intf;
import hikogui_concurrency;
import hikogui_coroutine;
import hikogui_time;

export namespace hi::inline v1 {

class awaitable_timer {
public:
    awaitable_timer(awaitable_timer &&) noexcept = default;
    awaitable_timer &operator=(awaitable_timer &&) noexcept = default;

    awaitable_timer(awaitable_timer const &other) noexcept : _deadline(other._deadline) {}

    awaitable_timer &operator=(awaitable_timer const &other) noexcept
    {
        _callback = nullptr;
        _deadline = other._deadline;
        return *this;
    }

    template<typename Duration>
    awaitable_timer(std::chrono::time_point<std::chrono::utc_clock, Duration> deadline) noexcept : _deadline(deadline)
    {
    }

    template<typename Rep, typename Period>
    awaitable_timer(std::chrono::duration<Rep, Period> period) noexcept : awaitable_timer(std::chrono::utc_clock::now() + period)
    {
    }

    [[nodiscard]] bool await_ready() const noexcept
    {
        return std::chrono::utc_clock::now() > _deadline;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept;

    void await_resume() const noexcept {}

private:
    utc_nanoseconds _deadline;
    callback<void()> _callback;
};

template<typename Rep, typename Period>
struct awaitable_cast<std::chrono::duration<Rep, Period>> {
    awaitable_timer operator()(std::chrono::duration<Rep, Period> const& rhs) const noexcept
    {
        return awaitable_timer{rhs};
    }
};

template<typename Duration>
struct awaitable_cast<std::chrono::time_point<std::chrono::utc_clock, Duration>> {
    awaitable_timer operator()(std::chrono::time_point<std::chrono::utc_clock, Duration> const& rhs) noexcept
    {
        return awaitable_timer{rhs};
    }
};

} // namespace hi::inline v1
