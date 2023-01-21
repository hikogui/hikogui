// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include "scoped_task.hpp"
#include "notifier.hpp"
#include "awaitable.hpp"
#include "awaitable_timer.hpp"
#include <coroutine>
#include <cstddef>
#include <type_traits>
#include <concepts>
#include <variant>
#include <tuple>
#include <chrono>

namespace hi::inline v1 {

/** An awaitable that waits for any of the given awaitables to complete.
 *
 * The return value of awaiting on `when_any` is a `std::variant` of the return
 * values of the given awaitables. If an awaitable has `void` as return type then
 * it will be converted to a `std::monotype` so that is can be used in the `std::variant`.
 *
 * The `index()` of the `std::variant` return type will match the index of the `when_any()`
 * constructor argument of the triggered awaitable.
 *
 * @tparam Ts Awaitable types.
 */
template<typename... Ts>
class when_any {
public:
    using value_type = std::variant<variant_decay_t<await_resume_result_t<Ts>>...>;

    /** Construct a `when_any` object from the given awaitables.
     *
     * The arguments may be of the following types:
     *  - An object which can be directly used as an awaitable. Having the member functions:
     *    `await_ready()`, `await_suspend()` and `await_resume()`.
     *  - An object that has a `operator co_await()` member function.
     *  - An object that has a `operator co_await()` free function.
     *  - An object that can be converted using the `awaitable_cast` functor.
     *
     * @param others The awaitable to wait for.
     */
    when_any(awaitable auto&&...others) noexcept :
        _awaiters(awaitable_cast<std::decay_t<decltype(others)>>{}(hi_forward(others))...)
    {
    }

    ~when_any() {}

    when_any(when_any&&) = delete;
    when_any(when_any const&) = delete;
    when_any& operator=(when_any&&) = delete;
    when_any& operator=(when_any const&) = delete;

    [[nodiscard]] constexpr bool await_ready() noexcept
    {
        static_assert(sizeof...(Ts) > 0);
        return _await_ready<0>();
    }

    void await_suspend(std::coroutine_handle<> const& handle) noexcept
    {
        static_assert(sizeof...(Ts) > 0);
        return _await_suspend<0>(handle);
    }

    value_type await_resume() noexcept
    {
        hi_assert(_value.has_value());
        return *_value;
    }

private:
    std::tuple<Ts...> _awaiters;
    std::tuple<scoped_task<await_resume_result_t<Ts>>...> _tasks;
    std::tuple<typename notifier<void(await_resume_result_t<Ts>)>::callback_token...> _task_cbts;
    std::optional<value_type> _value;

    template<awaitable_direct Awaiter>
    static scoped_task<await_resume_result_t<Awaiter>> _await_suspend_task(Awaiter& awaiter)
    {
        co_return co_await awaiter;
    }

    template<std::size_t I>
    void _destroy_tasks() noexcept
    {
        std::get<I>(_task_cbts) = {};
        std::get<I>(_tasks) = {};
        if constexpr (I + 1 < sizeof...(Ts)) {
            _destroy_tasks<I + 1>();
        }
    }

    template<std::size_t I>
    bool _await_ready() noexcept
    {
        auto& task = std::get<I>(_tasks) = _await_suspend_task(std::get<I>(_awaiters));

        if (task.done()) {
            using arg_type = await_resume_result_t<decltype(std::get<I>(_awaiters))>;

            if constexpr (std::is_same_v<arg_type, void>) {
                _value = value_type{std::in_place_index<I>, std::monostate{}};
            } else {
                _value = value_type{std::in_place_index<I>, task.value()};
            }
            _destroy_tasks<0>();
            return true;

        } else if constexpr (I + 1 < sizeof...(Ts)) {
            return _await_ready<I + 1>();

        } else {
            return false;
        }
    }

    template<std::size_t I>
    void _await_suspend(std::coroutine_handle<> const& handle) noexcept
    {
        using arg_type = await_resume_result_t<decltype(std::get<I>(_awaiters))>;

        if constexpr (std::is_same_v<arg_type, void>) {
            std::get<I>(_task_cbts) = std::get<I>(_tasks).subscribe(
                [this, handle]() {
                    this->_value = value_type{std::in_place_index<I>, std::monostate{}};
                    this->_destroy_tasks<0>();
                    handle.resume();
                },
                callback_flags::main | callback_flags::once);

        } else {
            std::get<I>(_task_cbts) = std::get<I>(_tasks).subscribe(
                [this, handle](arg_type const& arg) {
                    this->_value = value_type{std::in_place_index<I>, arg};
                    this->_destroy_tasks<0>();
                    handle.resume();
                },
                callback_flags::main | callback_flags::once);
        }

        if constexpr (I + 1 < sizeof...(Ts)) {
            _await_suspend<I + 1>(handle);
        }
    }

    template<typename... Args>
    friend class when_any;
};

template<awaitable... Others>
when_any(Others&&...) -> when_any<awaitable_cast_t<std::decay_t<Others>>...>;

} // namespace hi::inline v1
