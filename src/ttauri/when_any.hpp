// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "scoped_task.hpp"
#include "notifier.hpp"
#include "concepts.hpp"
#include "type_traits.hpp"
#include <coroutine>
#include <cstddef>
#include <type_traits>
#include <concepts>
#include <variant>
#include <tuple>

namespace tt::inline v1 {
namespace detail {

template<awaitable_direct T>
struct when_any_result_element {
    using type = std::conditional_t<std::is_same_v<await_resume_result_t<T>, void>, std::monostate, await_resume_result_t<T>>;
};

template<awaitable_direct T>
using when_any_result_element_t = when_any_result_element<T>::type;

} // namespace detail

/** Result of the `when_any` awaitable.
 */
template<typename... Ts>
class when_any_result {
public:
    using result_type = std::variant<std::monostate, detail::when_any_result_element_t<Ts>...>;
    using awaiter_type = std::variant<std::monostate, Ts...>;

    when_any_result() noexcept = default;
    when_any_result(when_any_result const &) noexcept = default;
    when_any_result(when_any_result &&) noexcept = default;
    when_any_result &operator=(when_any_result const &) noexcept = default;
    when_any_result &operator=(when_any_result &&) noexcept = default;

    template<std::size_t I, typename Awaiter, typename... Result>
    when_any_result(std::in_place_index_t<I>, Awaiter const &awaiter, Result &&...result) noexcept :
        _result{std::in_place_index<I + 1>, std::forward<Result>(result)...}, _awaiters{std::in_place_index<I + 1>, awaiter}
    {
        tt_axiom(_result.index() == _awaiters.index());
    }

    /** The index of the awaitable that was triggered.
     */
    [[nodiscard]] std::size_t index() const noexcept
    {
        return _result.index() - 1;
    }

    /** Comparison to check if the awaitable was the one that triggered `when_any`.
     */
    [[nodiscard]] bool operator==(awaitable auto const &rhs) const noexcept
    {
        return compare_equal<1>(awaitable_cast(rhs));
    }

    /** Get the value returned by the awaitable that triggered `when_any`.
     */
    template<typename T>
    friend auto &get(when_any_result const &) noexcept
    {
        return std::get<T>(_result);
    }

    /** Get the value returned by the awaitable that triggered `when_any`.
     */
    template<std::size_t I>
    friend auto &get(when_any_result const &) noexcept
    {
        return std::get<I + 1>(_result);
    }

private:
    result_type _result;
    awaiter_type _awaiters;

    template<size_t I>
    [[nodiscard]] bool compare_equal(awaitable_direct auto const &rhs) const noexcept
    {
        if (I != _awaiters.index()) {
            if constexpr (I + 1 < std::variant_size_v<awaiter_type>) {
                return compare_equal<I + 1>(rhs);
            } else {
                tt_no_default();
            }
        } else {
            using get_type = std::remove_cvref_t<decltype(get<I>(_awaiters))>;
            using cmp_type = std::remove_cvref_t<decltype(rhs)>;

            if constexpr (std::is_same_v<get_type, cmp_type>) {
                return std::get<I>(_awaiters) == rhs;
            } else {
                return false;
            }
        }
    }
};

/** An awaitable that waits for any of the given awaitables to complete.
 *
 */
template<typename... Ts>
class when_any {
public:
    using value_type = when_any_result<Ts...>;

    /** Construct a `when_any` object from the given awaitables.
     *
     * The arguments may be of the following types:
     *  - An object which can be directly used as an awaitable. Having the member functions:
     *    `await_ready()`, `await_suspend()` and `await_resume()` and `was_triggered()`.
     *  - An object that has a `operator co_await()` member function.
     *  - An object that has a `operator co_await()` free function.
     *
     * @param others The awaitable to wait for.
     */
    when_any(awaitable auto &&...others) noexcept : _awaiters(awaitable_cast(tt_forward(others))...) {}
    ~when_any() {}

    when_any(when_any &&) = delete;
    when_any(when_any const &) = delete;
    when_any &operator=(when_any &&) = delete;
    when_any &operator=(when_any const &) = delete;

    [[nodiscard]] constexpr bool await_ready() noexcept
    {
        static_assert(sizeof...(Ts) > 0);
        return _await_ready<0>();
    }

    void await_suspend(std::coroutine_handle<> const &handle) noexcept
    {
        static_assert(sizeof...(Ts) > 0);
        return _await_suspend<0>(handle);
    }

    value_type await_resume() noexcept
    {
        return _value;
    }

private:
    std::tuple<Ts...> _awaiters;
    std::tuple<scoped_task<await_resume_result_t<Ts>>...> _tasks;
    // std::tuple<typename notifier<detail::when_any_result_element_t<Ts>>::token_type...> _task_cbts;
    std::tuple<typename notifier<void(await_resume_result_t<Ts>)>::token_type...> _task_cbts;
    value_type _value;

    template<awaitable_direct Awaiter>
    static scoped_task<await_resume_result_t<Awaiter>> _await_suspend_task(Awaiter &awaiter)
    {
        co_return co_await awaiter;
    }

    template<std::size_t I>
    bool _await_ready() noexcept
    {
        auto &task = std::get<I>(_tasks) = _await_suspend_task(std::get<I>(_awaiters));

        if (task.completed()) {
            using arg_type = await_resume_result_t<decltype(std::get<I>(_awaiters))>;

            if constexpr (std::is_same_v<arg_type, void>) {
                _value = {std::in_place_index<I>, std::get<I>(_awaiters)};
            } else {
                _value = {std::in_place_index<I>, std::get<I>(_awaiters), task.value()};
            }
            return true;

        } else if constexpr (I + 1 < sizeof...(Ts)) {
            return _await_ready<I + 1>();

        } else {
            return false;
        }
    }

    template<std::size_t I>
    void _await_suspend(std::coroutine_handle<> const &handle) noexcept
    {
        using arg_type = await_resume_result_t<decltype(std::get<I>(_awaiters))>;

        if constexpr (std::is_same_v<arg_type, void>) {
            std::get<I>(_task_cbts) = std::get<I>(_tasks).subscribe([this, handle]() {
                this->_value = {std::in_place_index<I>, std::get<I>(_awaiters)};
                handle.resume();
            });

        } else {
            std::get<I>(_task_cbts) = std::get<I>(_tasks).subscribe([this, handle](arg_type const &arg) {
                this->_value = {std::in_place_index<I>, std::get<I>(_awaiters), arg};
                handle.resume();
            });
        }

        if constexpr (I + 1 < sizeof...(Ts)) {
            _await_suspend<I + 1>(handle);
        }
    }

    template<typename... Args>
    friend class when_any;
};

template<awaitable... Others>
when_any(Others &&...) -> when_any<awaitable_cast_type_t<Others>...>;

} // namespace tt::inline v1
