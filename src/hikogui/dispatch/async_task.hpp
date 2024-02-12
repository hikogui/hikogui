// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "progress.hpp"
#include "task.hpp"
#include "awaitable.hpp"
#include "awaitable_timer_intf.hpp"
#include <future>

hi_export_module(hikogui.dispatch.async_task);

hi_export namespace hi {
inline namespace v1 {

/** Run a function asynchronously as a co-routine task.
 *
 * @param func The function to be called.
 * @param args... The arguments forwarded to @a func.
 */
template<typename Func, typename... Args>
[[nodiscard]] std::invoke_result_t<Func, Args...> async_task(Func func, Args... args) requires(invocable_is_task_v<Func, Args...>)
{
    return func(args...);
}

/** Run a function asynchronously as a co-routine task.
 *
 * @note This will check every 15ms if the function is completed.
 * @param func The function to be called.
 * @param args... The arguments forwarded to @a func.
 */
template<typename Func, typename... Args>
[[nodiscard]] task<std::invoke_result_t<Func, Args...>> async_task(Func func, Args... args)
    requires(not invocable_is_task_v<Func, Args...>)
{
    using namespace std::literals;

    auto future = std::async(std::launch::async, [func = std::move(func), ... args = std::move(args)] {
        return func(args...);
    });

    if (not future.valid()) {
        throw std::future_error(std::future_errc::no_state);
    }

    while (true) {
        switch (future.wait_for(0s)) {
        case std::future_status::deferred:
            // This shouldn't really happen, this will cause the GUI to block.
            co_return future.get();

        case std::future_status::ready:
            co_return future.get();

        case std::future_status::timeout:
            co_await 15ms;
            break;

        default:
            hi_no_default();
        }
    }
}

/** Run a function asynchronously as a co-routine task.
 *
 * If the function @a func accepts the `std::stop_token` and/or
 * `hi::progress_token` then those arguments are passed to the function.
 *
 * @note This will call `async_task()`.
 * @param func The function to be called.
 * @param stop_token The stop token to optionally pass to the @a func.
 * @param progress_token The progress token to optionally pass to the @a func.
 * @param args... The arguments forwarded to @a func.
 */
template<typename Func, typename... Args>
[[nodiscard]] auto cancelable_async_task(Func func, std::stop_token stop_token, ::hi::progress_token progress_token, Args... args)
{
    if constexpr (std::is_invocable<Func, std::stop_token, ::hi::progress_token, Args...>) {
        return async_task(std::move(func), std::move(stop_token), std::move(progress_token), std::move(args)...);

    } else if constexpr (std::is_invocable<Func, ::hi::progress_token, Args...>) {
        return async_task(std::move(func), std::move(progress_token), std::move(args)...);

    } else if constexpr (std::is_invocable<Func, std::stop_token, Args...>) {
        return async_task(std::move(func), std::move(stop_token), std::move(args)...);

    } else if constexpr (std::is_invocable<Func, Args...>) {
        return async_task(std::move(func), std::move(args)...);

    } else {
        hi_static_no_default();
    }
}

} // namespace v1
}