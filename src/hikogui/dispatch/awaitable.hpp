// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <coroutine>
#include <type_traits>

hi_export_module(hikogui.dispatch.awaitable);

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
concept awaitable_with_co_await_member = requires(T a) {
    { a.operator co_await() } -> awaitable;
};

template<typename T>
concept awaitable_with_co_await_free_function = requires(T a) {
    { operator co_await(a) } -> awaitable;
};

template<typename T>
struct awaitable_cast;

template<awaitable T>
struct awaitable_cast<T> {
    template<typename RHS>
    decltype(auto) operator()(RHS&& rhs) const noexcept
    {
        return std::forward<RHS>(rhs);
    }
};

template<awaitable_with_co_await_member T>
struct awaitable_cast<T> {
    template<typename RHS>
    decltype(auto) operator()(RHS&& rhs) const noexcept
    {
        return std::forward<RHS>(rhs).operator co_await();
    }
};

template<awaitable_with_co_await_free_function T>
struct awaitable_cast<T> {
    template<typename RHS>
    decltype(auto) operator()(RHS&& rhs) const noexcept
    {
        return operator co_await(std::forward<RHS>(rhs));
    }
};

/** Check if type can be casted with `awaitable_cast` to an awaitable.
 */
template<typename T>
concept convertible_to_awaitable = requires(T rhs) { awaitable_cast<std::remove_cvref_t<T>>{}(rhs); };

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
