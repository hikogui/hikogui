// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <coroutine>
#include <type_traits>

hi_export_module(hikogui.coroutine.awaitable);

hi_export namespace hi::inline v1 {

/** Check if type can be directly co_await on.
 *
 * The type needs to have the following member functions:
 *  `await_ready()`, `await_suspend()` and `await_resume()`.
 */
template<typename T>
concept awaitable = requires(T a, std::coroutine_handle<> b) {
    // clang-format off
    { a.await_ready() } -> std::convertible_to<bool>;
    a.await_suspend(b);
    a.await_resume();
    // clang-format on
};

template<typename T>
struct awaitable_cast;

template<awaitable T>
struct awaitable_cast<T> {
    T operator()(T const &rhs) const noexcept
    {
        return rhs;
    }
};

template<typename T>
    requires requires(T const &rhs) { rhs.operator co_await(); }
struct awaitable_cast<T> {
    auto operator()(T const &rhs) const noexcept
    {
        return rhs.operator co_await();
    }
};

template<typename T>
    requires requires(T const &rhs) { operator co_await(rhs); }
struct awaitable_cast<T> {
    auto operator()(T const &rhs) const noexcept
    {
        return operator co_await(rhs);
    }
};

/** Check if type can be casted with `awaitable_cast` to an awaitable.
 */
template<typename T>
concept convertible_to_awaitable = requires(T const &rhs) { awaitable_cast<T>{}(rhs); };

/** Get the result type of an awaitable.
 *
 * This is type return type of the `await_resume()` member function.
 */
template<awaitable T>
struct await_resume_result {
    using type = decltype(std::declval<T>().await_resume());
};

/** Get the result type of an awaitable.
 *
 * This is type return type of the `await_resume()` member function.
 */
template<awaitable T>
using await_resume_result_t = await_resume_result<T>::type;

} // namespace hi::inline v1
