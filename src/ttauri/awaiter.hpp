// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include <coroutine>
#include <cstddef>
#include <type_traits>
#include <concepts>

namespace tt::inline v1 {

/** Check if type can be directly co_await on.
* 
* The type needs to have the following member functions:
*  `await_ready()`, `await_suspend()` and `await_resume()`.
*/
template<typename T>
concept awaitable_direct = requires(T a, std::coroutine_handle<> b)
{
    {a.await_ready()} -> std::convertible_to<bool>;
    a.await_suspend(b);
    a.await_resume();
};

/** Check if type can be indirectly co_await on.
 *
 * The type needs to implement member function `operator co_await()`.
 */
template<typename T>
concept awaitable_member = requires(T a)
{
    {a.operator co_await()};
};

/** Check if type can be indirectly co_await on.
 *
 * The type needs to implement free function `operator co_await()`.
 */
template<typename T>
concept awaitable_non_member = requires(T a)
{
    {operator co_await(static_cast<T &&>(a))};
};

/** Check if type can be directly or indirectly co_await on.
 *
 * The type needs to have the following member functions:
 *  `await_ready()`, `await_suspend()` and `await_resume()`.
 * Or implement `operator co_await()`.
 */
template<typename T>
concept awaitable = awaitable_direct<T> or awaitable_member<T> or awaitable_non_member<T>;

/** Check if type can be directly or indirectly co_await on.
 *
 * The type needs to have the following member functions:
 *  `await_ready()`, `await_suspend()` and `await_resume()`.
 * Or implement `operator co_await()`.
 */
template<typename T>
struct is_awaitable : std::false_type {
};

/** Check if type can be directly or indirectly co_await on.
 *
 * The type needs to have the following member functions:
 *  `await_ready()`, `await_suspend()` and `await_resume()`.
 * Or implement `operator co_await()`.
 */
template<awaitable T>
struct is_awaitable<T> : std::true_type {
};

/** Check if type can be directly or indirectly co_await on.
 *
 * The type needs to have the following member functions:
 *  `await_ready()`, `await_suspend()` and `await_resume()`.
 * Or implement `operator co_await()`.
 */
template<typename T>
constexpr bool is_awaitable_v = is_awaitable<T>::value;

/** Cast a object to an directly-awaitable object.
* 
* This function may use `operator co_await()` to retrieve the actual awaitable.
*/
decltype(auto) cast_awaitable(awaitable_direct auto &&rhs) noexcept
{
    return tt_forward(rhs);
}

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
decltype(auto) cast_awaitable(awaitable_member auto &&rhs) noexcept
{
    return tt_forward(rhs).operator co_await();
}

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
decltype(auto) cast_awaitable(awaitable_non_member auto &&rhs) noexcept
{
    return operator co_await(tt_forward(rhs));
}

/** Resolve the type that is directly-awaitable.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable type.
 */
template<awaitable T>
struct resolved_awaitable {
    using type = std::remove_cvref_t<decltype(cast_awaitable(std::declval<T>()))>;
};

/** Resolve the type that is directly-awaitable.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable type.
 */
template<awaitable T>
using resolved_awaitable_t = resolved_awaitable<T>::type;

/** Get the result type of an awaitable.
* 
* This is type return type of the `await_resume()` member function.
 */
template<awaitable_direct T>
struct await_resume_result {
    using type = decltype(std::declval<T>().await_resume());
};

/** Get the result type of an awaitable.
 *
 * This is type return type of the `await_resume()` member function.
 */
template<awaitable_direct T>
using await_resume_result_t = await_resume_result<T>::type;

namespace detail {
template<awaitable_direct T>
struct await_resume_result_variant {
    using type = std::conditional_t<std::is_same_v<await_resume_result_t<T>, void>, std::monostate, await_resume_result_t<T>>;
};

template<awaitable_direct T>
using await_resume_result_variant_t = await_resume_result_variant<T>::type;
}

/** Result of the `when_any` awaitable.
 */
template<typename... Ts>
class when_any_result {
public:
    using result_type = std::variant<detail::await_resume_result_variant_t<Ts>...>;
    using awaiter_type = std::variant<Ts...>;

    when_any_result(when_any_result const &) noexcept = default;
    when_any_result(when_any_result &&) noexcept = default;
    when_any_result &operator=(when_any_result const &) noexcept = default;
    when_any_result &operator=(when_any_result &&) noexcept = default;

    template<std::size_t I, typename Awaiter, typename Result>
    when_any_result(std::in_place_index_t<I>, Awaiter const &awaiter, Result &&result) noexcept :
        _result{std::in_place_index<I>, std::forward<Result>(result)}, _awaiters{std::in_place_index<I>, awaiter}
    {
    }

    template<std::size_t I, typename Awaiter>
    when_any_result(std::in_place_index_t<I>, Awaiter const &awaiter) noexcept :
        _result{std::in_place_index<I>}, _awaiters{std::in_place_index<I>, awaiter}
    {
    }

    /** The index of the awaitable that was triggered.
     */
    [[nodiscard]] std::size_t index() const noexcept
    {
        return _result.index();
    }

    /** Comparison to check if the awaitable was the one that triggered `when_any`.
     */
    [[nodiscard]] bool operator==(awaitable auto const &rhs) const noexcept
    {
        ttlet rhs_ = cast_awaitable(rhs);

        return std::visit(
            [&rhs_](auto const &lhs) -> bool {
                return lhs == rhs_;
            },
            _awaiters);
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
        return std::get<I>(_result);
    }

private:
    result_type _result;
    awaiter_type _awaiters;
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
    template<awaitable... Others>
    when_any(Others &&... others) noexcept :
        _awaiters(cast_awaitable(std::forward<Others>(others))...)
    {
    }

    [[nodiscard]] constexpr bool await_ready()
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> const &handle) noexcept
    {
        static_assert(sizeof...(Ts) > 0);
        return _await_suspend<0>(handle);
    }

    value_type await_resume() noexcept
    {
        static_assert(sizeof...(Ts) > 0);
        return _await_resume<0>();
    }

private:
    std::tuple<Ts...> _awaiters;

    template<std::size_t I>
    void _await_suspend(std::coroutine_handle<> const &handle) noexcept
    {
        std::get<I>(_awaiters).await_suspend(handle);
        if constexpr (I + 1 < sizeof...(Ts)) {
            _await_suspend<I + 1>(handle);
        }
    }

    template<std::size_t I>
    value_type _await_resume() noexcept
    {
        auto &awaiter = std::get<I>(_awaiters);
        if (awaiter.was_triggered()) {
            if constexpr (std::is_same_v<decltype(awaiter.await_resume()), void>) {
                awaiter.await_resume();
                return value_type{std::in_place_index<I>, awaiter};
            } else {
                return value_type{std::in_place_index<I>, awaiter, awaiter.await_resume()};
            }

        } else if constexpr (I + 1 < sizeof...(Ts)) {
            return _await_resume<I + 1>();

        } else {
            // At least one of the awaiters must be triggered.
            tt_no_default();
        }
    }

    template<typename... Args>
    friend class when_any;
};

template<awaitable... Others>
when_any(Others &&...) -> when_any<resolved_awaitable_t<Others>...>;

} // namespace tt::inline v1
