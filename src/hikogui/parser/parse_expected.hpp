

#pragma once

#include "../macros.hpp"
#include <expected>
#include <optional>
#include <variant>

hi_export_module(hikogui.parser : parse_expected);

hi_export namespace hi {
inline namespace v1 {

template<typename T, typename E>
class parse_expected {
public:
    using value_type = T;
    using error_type = E;

    constexpr parse_expected(parse_expected const&) noexcept = default;
    constexpr parse_expected(parse_expected&&) noexcept = default;
    constexpr parse_expected& operator=(parse_expected const&) noexcept = default;
    constexpr parse_expected& operator=(parse_expected&&) noexcept = default;

    constexpr parse_expected() noexcept = default;

    constexpr parse_expected(std::nullopt_t) noexcept _v{std::monostate{}} {}

    template<typename Arg = T>
    constexpr explicit(not std::is_convertible_v<Arg, T>) parse_expected(Arg&& arg) noexcept : _v{std::forward<Arg>(arg)}
    {
    }

    template<typename Arg>
    constexpr explicit(not std::is_convertible_v<const Arg&, E>) parse_expected(std::unexpected<Arg> const& arg) noexcept :
        _v{arg}
    {
    }

    constexpr parse_expected &operator=(std::nullopt_t) noexcept
    {
        _v.template emplace<0>();
        return *this;
    }

    template<typename Arg = T>
    constexpr parse_expected &operator=(Arg &&arg) noexcept
    {
        _v.template emplace<1>(std::forward<Arg>(arg));
        return *this;
    }

    template<typename Arg = E>
    constexpr parse_expected &operator=(std::unexpected<Arg> const &arg) noexcept
    {
        _v.template emplace<2>(arg);
        return *this;
    }

    template<typename Arg = E>
    constexpr parse_expected &operator=(std::unexpected<Arg> &&arg) noexcept
    {
        _v.template emplace<2>(std::move(arg));
        return *this;
    }

    template<typename... Args>
    constexpr value_type &emplace(Args &&... args) noexcept
    {
        return _v.template emplace<1>(std::forward<Args>(args)...);
    }

    [[nodiscard]] constexpr bool has_value() const noexcept
    {
        return _v.index() == 1;
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    [[nodiscard]] constexpr bool has_error() const noexcept
    {
        return _v.index() == 2;
    }

    [[nodiscard]] constexpr value_type const& operator*() const noexcept
    {
        hi_axiom(_v.index() == 1);
        return std::get<1>(_v);
    }

    [[nodiscard]] constexpr value_type& operator*() noexcept
    {
        hi_axiom(_v.index() == 1);
        return std::get<1>(_v);
    }

    [[nodiscard]] constexpr value_type const* operator->() const noexcept
    {
        hi_axiom(_v.index() == 1);
        return std::addressof(::get<1>(_v));
    }

    [[nodiscard]] constexpr value_type* operator->() noexcept
    {
        hi_axiom(_v.index() == 1);
        return std::addressof(std::get<1>(_v));
    }

    [[nodiscard]] constexpr error_type const& error() const noexcept
    {
        hi_axiom(_v.index() == 2);
        return std::get<2>(_v);
    }

    [[nodiscard]] constexpr error_type& error() noexcept
    {
        hi_axiom(_v.index() == 2);
        return std::get<2>(_v);
    }

private:
    std::variant<std::monostate, T, E> _v;
};

} // namespace v1
}
