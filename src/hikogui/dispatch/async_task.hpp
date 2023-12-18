// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "task.hpp"
#include <future>

hi_export_module(hikogui.dispatch.async_task);

hi_export namespace hi { inline namespace v1 {

/** Run a function asynchronously as a co-routine task.
 * 
 * @note This will check every 15ms if the function is completed.
 * @param func The function to be called.
 * @param args... The arguments forwarded to @a func.
*/
template<typename Func, typename... Args>
[[nodiscard]] task<std::invoke_result<Func, Args...>> async_task(Func const& func, Args &&... args)
{
    auto future = std::async(
        std::launch::async,
        [&] {
            std::forward<Func>(func)(std::forward<Args>(args)...);
        });

    if (not future.valid()) {
        throw std::future_error(std::future_errc::no_state);
    }

    using namespace std::literals;
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

template<typename Func, typename... Args>
[[nodiscard]] constexpr auto make_async_task(Func &&func) noexcept
{
    return [function = std::forward<Func>(func)](Args const &...args) -> task<std::invoke_result<Func, Args...>> { return function(args...); };
}

}}