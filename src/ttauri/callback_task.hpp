// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include <coroutine>
#include <type_traits>

namespace tt::inline v1 {

template<typename T>
class callback_task;

template<typename T>
struct callback_task_promise_base {
    T _value;
    notifier<void(value_type)> _notifier;

    void return_void() noexcept
    {
        tt_no_default();
    }

    void return_value(std::convertible_to<T> auto &&value) noexcept
    {
        _value = tt_forward(value);
    }

    std::suspend_never final_suspend() noexcept
    {
        _notifier(_value);
        return {};
    }
};

template<>
struct callback_task_promise_base<void> {
    notifier<void()> _notifier;

    void return_void() noexcept {}

    std::suspend_never final_suspend() noexcept
    {
        _notifier();
        return {};
    }
};

template<typename T>
struct callback_task_promise : callback_task_promise_base<T> {
    using value_type = T;
    using handle_type = std::coroutine_handle<callback_task_promise<value_type>>;
    using callback_task_type = callback_task<value_type>;

    void unhandled_exception()
    {
        throw;
    }

    callback_task<value_type> get_return_object()
    {
        return callback_task_type{handle_type::from_promise(*this)};
    }

    /** Before we enter the coroutine, allow the caller to set the callback.
     */
    std::suspend_always initial_suspend() noexcept
    {
        return {};
    }
};

/** Co-routine callback_task.
 * 
 * @tparam T The type returned by co_return.
 */
template<typename T = void>
class callback_task {
public:
    using value_type = T;
    using promise_type = callback_task_promise<value_type>;
    using handle_type = std::coroutine_handle<promise_type>;

    explicit callback_task(handle_type coroutine) : _coroutine(coroutine) {}

    callback_task() = default;
    callback_task(callback_task const &) = delete;
    callback_task(callback_task &&) = delete;
    callback_task &operator=(callback_task const &) = delete;
    callback_task &operator=(callback_task &&) = delete;

    auto resume(std::invocable<T> auto &&f) noexcept
    {
        auto tmp = _coroutine.promise()._notifier.subscribe(tt_forward(f));
        _coroutine.resume();
        return tmp;
    }

private:
    handle_type _coroutine;
};

} // namespace tt::inline v1
