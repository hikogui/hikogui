// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <optional>
#include <functional>

namespace hi { inline namespace v1 {

template<typename T>
class cache {
public:
    using value_type = T;

    ~cache() = default;
    constexpr cache(cache const&) noexcept = default;
    constexpr cache(cache&&) noexcept = default;
    constexpr cache& operator=(cache const&) noexcept = default;
    constexpr cache& operator=(cache&&) noexcept = default;
    constexpr cache() noexcept = default;

    constexpr cache& operator=(forward_of<value_type()> auto&& func) noexcept
    {
        _loader = hi_forward(func);
        return *this;
    }

    constexpr void reset() const noexcept
    {
        _value = std::nullopt;
    }

    [[nodiscard]] constexpr value_type const& load() const noexcept
    {
        if (not _value) {
            hi_axiom(_loader);
            _value = _loader();
        }
        return *_value;
    }

    [[nodiscard]] constexpr value_type const& reload() const noexcept
    {
        hi_axiom(_loader);
        _value = _loader();
        return *_value;
    }

    constexpr operator value_type const &() const noexcept
    {
        return load();
    }

    [[nodiscard]] constexpr value_type const& operator*() const noexcept
    {
        return load();
    }

    [[nodiscard]] constexpr value_type *operator->() const noexcept
    {
        return &load();
    }

#define HI_X(op) \
    [[nodiscard]] constexpr friend auto operator op(cache const& lhs, cache const& rhs) noexcept \
        requires requires { operator op(*lhs, *rhs); } \
    { \
        return operator op(*lhs, *rhs); \
    } \
\
    [[nodiscard]] constexpr friend auto operator op(cache const& lhs, auto const& rhs) noexcept \
        requires requires { operator op(*lhs, rhs); } \
    { \
        return operator op(*lhs, rhs); \
    } \
\
    [[nodiscard]] constexpr friend auto operator op(auto const& lhs, cache const& rhs) noexcept \
        requires requires { operator op(lhs, *rhs); } \
    { \
        return operator op(lhs, *rhs); \
    }

    HI_X(+)
    HI_X(-)
    HI_X(*)
    HI_X(/)
    HI_X(%)
    HI_X(==)
    HI_X(<=>)
    HI_X(!=)
    HI_X(<)
    HI_X(>)
    HI_X(<=)
    HI_X(>=)
    HI_X(<<)
    HI_X(>>)
#undef HI_X

#define HI_X(op) \
    [[nodiscard]] constexpr friend auto operator op(cache const& rhs) noexcept \
        requires requires { operator op(*rhs); } \
    { \
        return operator op(*rhs); \
    }

    HI_X(-)
    HI_X(+)
    HI_X(~)
#undef HI_X

    [[nodiscard]] constexpr auto operator[](auto const &rhs) noexcept
    {
        return (*this)[rhs];
    }

    [[nodiscard]] constexpr auto operator()(auto &&... rhs) noexcept
    {
        return (*this)(hi_forward(rhs)...);
    }

private:
    mutable std::optional<value_type> _value;
    std::function<value_type()> _loader;
};

}} // namespace hi::v1
