// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <coroutine>
#include <type_traits>

namespace hi::inline v1 {

/** Check if type can be directly co_await on.
 *
 * The type needs to have the following member functions:
 *  `await_ready()`, `await_suspend()` and `await_resume()`.
 */
template<typename T>
concept awaitable_direct = requires(T a, std::coroutine_handle<> b)
{
    // clang-format off
    { a.await_ready() } -> std::convertible_to<bool>;
    a.await_suspend(b);
    a.await_resume();
    // clang-format on
};

/** Check if type can be indirectly co_await on.
 *
 * The type needs to implement member function `operator co_await()`.
 */
template<typename T>
concept awaitable_member = requires(T a)
{
    a.operator co_await();
};

/** Check if type can be indirectly co_await on.
 *
 * The type needs to implement free function `operator co_await()`.
 */
template<typename T>
concept awaitable_non_member = requires(T a)
{
    operator co_await(static_cast<T&&>(a));
};

/** A functor for casting a type to an awaitable.
* 
* This class holds a `value_type` for the type that is returned by the functor.
* And it will have a `operator()` which will return a value_type and accepts
* a single forwarding argument for the object to convert.
* 
* @note You may add a specialization for `hi::awaitable_cast` for your own types.
 */
template<typename T>
struct awaitable_cast {
    using type = void;
};

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
template<awaitable_direct T>
struct awaitable_cast<T> {
    using type = std::decay_t<T>;

    [[nodiscard]] type operator()(auto&& rhs) const noexcept
    {
        return hi_forward(rhs);
    }
};

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
template<awaitable_member T>
struct awaitable_cast<T> {
    using type = std::decay_t<decltype(std::declval<T>().operator co_await())>;

    [[nodiscard]] type operator()(auto&& rhs) const noexcept
    {
        return hi_forward(rhs).operator co_await();
    }
};

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
template<awaitable_non_member T>
struct awaitable_cast<T> {
    using type = std::decay_t<decltype(operator co_await(std::declval<T>()))>;

    [[nodiscard]] type operator()(auto&& rhs) const noexcept
    {
        return operator co_await(hi_forward(rhs));
    }
};

/** Resolve the type that is directly-awaitable.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable type.
 */
template<typename T>
using awaitable_cast_t = hi_typename awaitable_cast<T>::type;

/** Check if the type can be co_awaited on after conversion with `awaitable_cast`.
 *
 * The type needs to do one of the following:
 *  - Has the following functions: `await_ready()`, `await_suspend()` and `await_resume()`.
 *  - Has a member function `operator co_await()`.
 *  - Has a non-member function `operator co_await()`.
 *  - Has a template specialization of `hi::awaitable_cast`.
 */
template<typename T>
concept awaitable = not std::is_same_v<awaitable_cast_t<T>, void>;

/** Get the result type of an awaitable.
 *
 * This is type return type of the `await_resume()` member function.
 */
template<typename T>
struct await_resume_result {
    using type = decltype(std::declval<T>().await_resume());
};

/** Get the result type of an awaitable.
 *
 * This is type return type of the `await_resume()` member function.
 */
template<typename T>
using await_resume_result_t = hi_typename await_resume_result<T>::type;

}
