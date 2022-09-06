// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "i18n/translate.hpp"
#include "unicode/gstring.hpp"
#include "concepts.hpp"
#include <variant>
#include <string>

namespace hi::inline v1 {

/** An image, in different formats.
 */
class text {
public:
    constexpr text(forward_of<std::string> auto&& text) : _text(hi_forward(text)) {}
    constexpr text(forward_of<gstring> auto&& text) : _text(hi_forward(text)) {}
    constexpr text(forward_of<translate> auto&& text) : _text(hi_forward(text)) {}

    constexpr text() noexcept : _text(std::monostate{}) {}

    constexpr text(text const&) noexcept = default;
    constexpr text(text&&) noexcept = default;
    constexpr text& operator=(text const&) noexcept = default;
    constexpr text& operator=(text&&) noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return std::holds_alternative<std::monostate>(_text);
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr friend bool operator==(text const& lhs, text const& rhs) noexcept = default;

    template<typename T>
    [[nodiscard]] constexpr friend bool holds_alternative(hi::text const& rhs) noexcept
    {
        return std::holds_alternative<T>(rhs._text);
    }

    template<typename T>
    [[nodiscard]] constexpr friend T const& get(hi::text const& rhs) noexcept
    {
        return std::get<T>(rhs._text);
    }

    template<typename T>
    [[nodiscard]] constexpr friend T& get(hi::text const& rhs) noexcept
    {
        return std::get<T>(rhs._text);
    }

    template<typename T>
    [[nodiscard]] constexpr friend std::add_pointer_t<T const> get_if(hi::text const *rhs) noexcept
    {
        return std::get_if<T>(&rhs->_text);
    }

    template<typename T>
    [[nodiscard]] constexpr friend std::add_pointer_t<T> get_if(hi::text const *rhs) noexcept
    {
        return std::get_if<T>(&rhs->_text);
    }

    [[nodiscard]] constexpr friend std::string to_string(hi::text const &rhs) noexcept
    {
        // clang-format off
        return std::visit(
            overloaded{
                [](std::monostate const &) { return std::string{}; },
                [](std::string const &x) { return x; },
                [](gstring const &x) { return hi::to_string(x); },
                [](translate const &x) { return x(); }
            },
            rhs._text);
        // clang-format on
    }

    [[nodiscard]] constexpr friend std::wstring to_wstring(hi::text const& rhs) noexcept
    {
        // clang-format off
        return std::visit(
            overloaded{
                [](std::monostate const &) { return std::wstring{}; },
                [](std::string const &x) { return hi::to_wstring(x); },
                [](gstring const &x) { return hi::to_wstring(hi::to_string(x)); },
                [](translate const &x) { return hi::to_wstring(x()); }
            },
            rhs._text);
        // clang-format on
    }

private:
    using text_type = std::variant<std::monostate, std::string, gstring, translate>;

    text_type _text;
};

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::text, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::text const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};
