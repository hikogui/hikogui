// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "notifier.hpp"
#include <coroutine>
#include <type_traits>

namespace tt::inline v1 {

template<typename T>
class delayed_task;

namespace detail {

template<typename T>
struct delayed_task_promise_base {
    using value_type = T;

    notifier<void(value_type)> _notifier;
    value_type _value;

    void return_value(std::convertible_to<value_type> auto &&value) noexcept
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
struct delayed_task_promise_base<void> {
    notifier<void()> _notifier;

    void return_void() noexcept {}

    std::suspend_never final_suspend() noexcept
    {
        _notifier();
        return {};
    }
};

template<typename T>
struct delayed_task_promise : delayed_task_promise_base<T> {
    using value_type = T;
    using handle_type = std::coroutine_handle<delayed_task_promise<value_type>>;
    using delayed_task_type = delayed_task<value_type>;

    void unhandled_exception()
    {
        throw;
    }

    delayed_task<value_type> get_return_object()
    {
        return delayed_task_type{handle_type::from_promise(*this)};
    }

    /** Before we enter the coroutine, allow the caller to set the callback.
     */
    std::suspend_always initial_suspend() noexcept
    {
        return {};
    }
};

} // namespace detail

/** A delayed_task.
 *
 * Like the `tt::task` instance this implements a asynchronous co-routine task.
 * This delayed variant will immediately return to the caller until `delayed_task::resume()` is
 * called on this. This will allow this object to be assigned callbacks to trigger when co_return
 * is called.
 *
 * The `delayed_task` object needs to be held by the caller until the co-routine returns.
 * If the `delayed_task` object is destroyed, the co-routine will be destroyed as well.
 *
 * @tparam T The type returned by co_return.
 */
template<typename T = void>
class delayed_task {
public:
    using value_type = T;
    using promise_type = detail::delayed_task_promise<value_type>;
    using handle_type = std::coroutine_handle<promise_type>;

    explicit delayed_task(handle_type coroutine) : _coroutine(coroutine) {}

    ~delayed_task()
    {
    }

    delayed_task() = default;
    delayed_task(delayed_task const &) = delete;
    delayed_task &operator=(delayed_task const &) = delete;

    delayed_task(delayed_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
    }

    delayed_task &operator=(delayed_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        return *this;
    }

    /** Resume the co-routine.
     *
     * The co-routine initially suspends, which allows a callback to be
     * attached to the co-routine which will be called the co-routine returns.
     *
     * @param callback The callback to be called when the co-routine returns.
     * @return A callback-token used to track the lifetime of the callback.
     */
    auto resume(std::invocable<value_type> auto &&callback) noexcept
    {
        auto tmp = _coroutine.promise()._notifier.subscribe(tt_forward(callback));
        _coroutine.resume();
        return tmp;
    }

    /** Resume the co-routine.
     */
    auto resume(std::invocable<> auto &&callback) noexcept requires(std::is_same_v<value_type, void>)
    {
        auto tmp = _coroutine.promise()._notifier.subscribe(tt_forward(callback));
        _coroutine.resume();
        return tmp;
    }

private:
    handle_type _coroutine;
};

} // namespace tt::inline v1
