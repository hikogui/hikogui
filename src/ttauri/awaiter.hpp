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

template<typename T>
concept awaitable_has_ready = requires(T a)
{
    {
        a.await_ready()
        } -> std::convertible_to<bool>;
};

template<typename T>
concept awaitable_has_suspend = requires(T a, std::coroutine_handle<> b)
{
    {a.await_suspend(b)};
};

template<typename T>
concept awaitable_has_resume = requires(T a)
{
    {a.await_resume()};
};

template<typename T>
concept awaitable_direct = awaitable_has_ready<T> and awaitable_has_suspend<T> and awaitable_has_resume<T>;

template<typename T>
concept awaitable_member = requires(T a)
{
    {a.operator co_await()};
};

template<typename T>
concept awaitable_non_member = requires(T a)
{
    {operator co_await(static_cast<T &&>(a))};
};

template<typename T>
concept awaitable = awaitable_direct<T> or awaitable_member<T> or awaitable_non_member<T>;

template<typename T>
struct is_awaitable : std::false_type {
};

template<awaitable T>
struct is_awaitable<T> : std::true_type {
};

template<typename T>
constexpr bool is_awaitable_v = is_awaitable<T>::value;

decltype(auto) cast_awaitable(awaitable_direct auto &&rhs) noexcept
{
    return tt_forward(rhs);
}

decltype(auto) cast_awaitable(awaitable_member auto &&rhs) noexcept
{
    return tt_forward(rhs).operator co_await();
}

decltype(auto) cast_awaitable(awaitable_non_member auto &&rhs) noexcept
{
    return operator co_await(tt_forward(rhs));
}

template<awaitable T>
struct resolved_awaitable {
    using type = std::remove_cvref_t<decltype(cast_awaitable(std::declval<T>()))>;
};

template<awaitable T>
using resolved_awaitable_t = resolved_awaitable<T>::type;

template<awaitable_direct T>
struct await_resume_result {
    using type = decltype(std::declval<T>().await_resume());
};

template<awaitable_direct T>
using await_resume_result_t = await_resume_result<T>::type;

template<awaitable_direct T>
struct await_resume_result_variant {
    using type = std::conditional_t<std::is_same_v<await_resume_result_t<T>, void>, std::monostate, await_resume_result_t<T>>;
};

template<awaitable_direct T>
using await_resume_result_variant_t = await_resume_result_variant<T>::type;

template<typename... Ts>
class when_any_result {
public:
    using result_type = std::variant<await_resume_result_variant_t<Ts>...>;
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

    [[nodiscard]] std::size_t index() const noexcept
    {
        return _result.index();
    }

    [[nodiscard]] bool operator==(awaitable auto const &rhs) const noexcept
    {
        ttlet rhs_ = cast_awaitable(rhs);

        return std::visit(
            [&rhs_](auto const &lhs) -> bool {
                return lhs == rhs_;
            },
            _awaiters);
    }

    template<typename T>
    friend auto &get(when_any_result const &) noexcept
    {
        return std::get<T>(_result);
    }

    template<std::size_t I>
    friend auto &get(when_any_result const &) noexcept
    {
        return std::get<I>(_result);
    }

private:
    result_type _result;
    awaiter_type _awaiters;
};

template<typename... Ts>
class when_any {
public:
    using value_type = when_any_result<Ts...>;

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
