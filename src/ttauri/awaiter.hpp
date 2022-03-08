// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <coroutine>
#include <cstddef>

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

template<awaitable T>
struct cast_awaitable {
};

template<awaitable_direct T>
struct cast_awaitable<T> {
    auto operator()(T const &a) const noexcept
    {
        return a;
    }
};

template<awaitable_member T>
struct cast_awaitable<T> {
    auto operator()(T const &a) const noexcept
    {
        return a.operator co_await();
    }
};

template<awaitable_non_member T>
struct cast_awaitable<T> {
    auto operator()(T const &a) const noexcept
    {
        return operator co_await(a);
    }
};

template<awaitable_direct T>
struct awaitable_variant_element {
    using actual_type = decltype(T{}.await_resume());
    using type = std::conditional_t<std::is_same_v<actual_type, void>, std::monostate, actual_type>;
};

template<awaitable_direct T>
using awaitable_variant_element_t = awaitable_variant_element<T>::type;

template<typename... Ts>
class awaiter_or_result {
public:
    using result_type = std::variant<awaitable_variant_element_t<Ts>...>;
    using awaiter_type = std::variant<Ts...>;

    awaiter_or_result(awaiter_or_result const &) noexcept = default;
    awaiter_or_result(awaiter_or_result &&) noexcept = default;
    awaiter_or_result &operator=(awaiter_or_result const &) noexcept = default;
    awaiter_or_result &operator=(awaiter_or_result &&) noexcept = default;

    template<std::size_t I, typename Awaiter, typename Result>
    awaiter_or_result(std::in_place_index_t<I>, Awaiter const &awaiter, Result &&result) noexcept :
        _result{std::in_place_index<I>, std::forward<Result>(result)}, _awaiters{std::in_place_index<I>, awaiter}
    {
    }

    template<std::size_t I, typename Awaiter>
    awaiter_or_result(std::in_place_index_t<I>, Awaiter const &awaiter) noexcept :
        _result{std::in_place_index<I>}, _awaiters{std::in_place_index<I>, awaiter}
    {
    }

    [[nodiscard]] std::size_t index() const noexcept
    {
        return _result.index();
    }

    [[nodiscard]] bool operator==(awaitable auto const &rhs) const noexcept
    {
        ttlet rhs_ = cast_awaitable<decltype(rhs)>{}(rhs);

        return std::visit(
            [&rhs_](auto const &lhs) -> bool {
                return lhs == rhs_;
            },
            _awaiters);
    }

    template<typename T>
    friend auto &get(awaiter_or_result const &) noexcept
    {
        return std::get<T>(_result);
    }

    template<std::size_t I>
    friend auto &get(awaiter_or_result const &) noexcept
    {
        return std::get<I>(_result);
    }

private:
    result_type _result;
    awaiter_type _awaiters;
};

template<typename... Ts>
class awaiter_or {
public:
    using value_type = awaiter_or_result<Ts...>;

    template<typename... LhsTs, typename... RhsTs>
    awaiter_or(awaiter_or<LhsTs...> const &lhs, awaiter_or<RhsTs...> const &rhs) noexcept :
        _awaiters(std::tuple_cat(lhs._awaiters, rhs._awaiters))
    {
    }

    template<typename... LhsTs, awaitable_direct Rhs>
    awaiter_or(awaiter_or<LhsTs...> const &lhs, Rhs const &rhs) noexcept :
        _awaiters(std::tuple_cat(lhs._awaiters, std::tuple{rhs}))
    {
    }

    template<awaitable_direct Lhs, typename... RhsTs>
    awaiter_or(Lhs const &lhs, awaiter_or<RhsTs...> const &rhs) noexcept :
        _awaiters(std::tuple_cat(std::tuple{lhs}, rhs._awaiters))
    {
    }

    template<awaitable_direct Lhs, awaitable_direct Rhs>
    awaiter_or(Lhs const &lhs, Rhs const &rhs) noexcept : _awaiters{lhs, rhs}
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
    friend class awaiter_or;
};

template<typename... LhsTs, typename... RhsTs>
awaiter_or(awaiter_or<LhsTs...> const &lhs, awaiter_or<RhsTs...> const &rhs) -> awaiter_or<LhsTs..., RhsTs...>;

template<typename... LhsTs, awaitable_direct Rhs>
awaiter_or(awaiter_or<LhsTs...> const &lhs, Rhs const &rhs) -> awaiter_or<LhsTs..., Rhs>;

template<awaitable_direct Lhs, typename... RhsTs>
awaiter_or(Lhs const &lhs, awaiter_or<RhsTs...> const &rhs) -> awaiter_or<Lhs, RhsTs...>;

template<awaitable_direct Lhs, awaitable_direct Rhs>
awaiter_or(Lhs const &lhs, Rhs const &rhs) -> awaiter_or<Lhs, Rhs>;

template<awaitable Lhs, awaitable Rhs>
[[nodiscard]] auto operator||(Lhs const &lhs, Rhs const &rhs) noexcept
{
    return awaiter_or{cast_awaitable<Lhs>{}(lhs), cast_awaitable<Rhs>{}(rhs)};
}

} // namespace tt::inline v1
