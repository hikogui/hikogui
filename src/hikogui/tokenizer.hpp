// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "strings.hpp"
#include "small_vector.hpp"
#include "utility.hpp"
#include "decimal.hpp"
#include "exception.hpp"
#include "parse_location.hpp"
#include "charconv.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <charconv>
#include <array>

namespace hi::inline v1 {

enum class tokenizer_name_t : uint8_t {
    NotAssigned,
    ErrorInvalidCharacter,
    ErrorEOTInBlockComment,
    ErrorEOTInString,
    ErrorLFInString,

    Name,
    StringLiteral,
    IntegerLiteral,
    DateLiteral,
    TimeLiteral,
    FloatLiteral,
    Operator, // Operator, or bracket, or other literal text.
    End
};

constexpr char const *to_const_string(tokenizer_name_t name) noexcept
{
    switch (name) {
    case tokenizer_name_t::NotAssigned:
        return "NotAssigned";
    case tokenizer_name_t::ErrorInvalidCharacter:
        return "ErrorInvalidCharacter";
    case tokenizer_name_t::ErrorEOTInBlockComment:
        return "ErrorEOTInBlockComment";
    case tokenizer_name_t::ErrorEOTInString:
        return "ErrorEOTInString";
    case tokenizer_name_t::ErrorLFInString:
        return "ErrorLFInString";
    case tokenizer_name_t::Name:
        return "Name";
    case tokenizer_name_t::StringLiteral:
        return "StringLiteral";
    case tokenizer_name_t::IntegerLiteral:
        return "IntegerLiteral";
    case tokenizer_name_t::DateLiteral:
        return "DateLiteral";
    case tokenizer_name_t::TimeLiteral:
        return "TimeLiteral";
    case tokenizer_name_t::FloatLiteral:
        return "FloatLiteral";
    case tokenizer_name_t::Operator:
        return "Operator";
    case tokenizer_name_t::End:
        return "End";
    default:
        hi_no_default();
    }
}

inline std::ostream& operator<<(std::ostream& lhs, tokenizer_name_t rhs)
{
    return lhs << to_const_string(rhs);
}

struct token_t;
} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::tokenizer_name_t, CharT> : std::formatter<char const *, CharT> {
    auto format(hi::tokenizer_name_t const& t, auto& fc) const
    {
        return std::formatter<char const *, CharT>{}.format(hi::to_const_string(t), fc);
    }
};

template<typename CharT>
struct std::formatter<hi::token_t, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::token_t const& t, auto& fc) const -> decltype(std::formatter<char const *, CharT>{}.format("", fc));
};

namespace hi::inline v1 {

struct token_t {
    tokenizer_name_t name = tokenizer_name_t::NotAssigned;
    std::string value;
    parse_location location = {};
    bool is_binary = false;
    int precedence = 0;

    constexpr ~token_t() = default;
    constexpr token_t() noexcept = default;
    constexpr token_t(token_t const& other) noexcept = default;
    constexpr token_t(token_t&& other) noexcept = default;
    constexpr token_t& operator=(token_t const& other) noexcept = default;
    constexpr token_t& operator=(token_t&& other) noexcept = default;

    token_t(tokenizer_name_t name, std::string value) noexcept :
        name(name), value(std::move(value)), location(), is_binary(false), precedence(0)
    {
    }

    operator bool() const noexcept
    {
        return name != tokenizer_name_t::NotAssigned;
    }

    explicit operator long double() const
    {
        try {
            return std::stold(value);

        } catch (...) {
            throw parse_error(std::format("Could not convert token {} to long double", *this));
        }
    }

    explicit operator double() const
    {
        try {
            return std::stod(value);

        } catch (...) {
            throw parse_error(std::format("Could not convert token {} to double", *this));
        }
    }

    explicit operator float() const
    {
        try {
            return std::stof(value);

        } catch (...) {
            throw parse_error(std::format("Could not convert token {} to float", *this));
        }
    }

    template<std::integral T>
    explicit operator T() const
    {
        try {
            return hi::from_string<T>(value);

        } catch (...) {
            throw parse_error(std::format("Could not convert token {} to {}", *this, typeid(T).name()));
        }
    }

    explicit operator std::string() const noexcept
    {
        return value;
    }

    explicit operator decimal() const
    {
        return decimal{value};
    }

    explicit operator std::chrono::year_month_day() const
    {
        hilet parts = split(value, '-');
        if (parts.size() != 3) {
            throw parse_error("Expect date to be in the format YYYY-MM-DD");
        }

        hilet year = std::chrono::year{stoi(parts[0])};
        hilet month = std::chrono::month{narrow_cast<unsigned int>(stoi(parts[1]))};
        hilet day = std::chrono::day{narrow_cast<unsigned int>(stoi(parts[2]))};
        return {year, month, day};
    }

    std::string repr() const noexcept
    {
        std::string r = to_const_string(name);
        if (value.size() > 0) {
            r += '\"';
            r += value;
            r += '\"';
        }
        return r;
    }

    friend inline std::ostream& operator<<(std::ostream& lhs, token_t const& rhs)
    {
        return lhs << rhs.repr();
    }

    [[nodiscard]] friend bool operator==(token_t const& lhs, token_t const& rhs) noexcept
    {
        return (lhs.name == rhs.name) && (lhs.value == rhs.value);
    }

    [[nodiscard]] friend bool operator==(token_t const& lhs, tokenizer_name_t const& rhs) noexcept
    {
        return lhs.name == rhs;
    }

    [[nodiscard]] friend bool operator==(token_t const& lhs, const char *rhs) noexcept
    {
        return lhs.value == rhs;
    }
};

using token_vector = std::vector<hi::token_t>;
using token_iterator = typename token_vector::iterator;

template<typename T>
struct parse_result {
    bool found;
    T value;
    token_iterator next_token;

    parse_result() noexcept : found(false), value(), next_token() {}

    parse_result(T const& value, token_iterator next_token) : found(true), value(value), next_token(next_token) {}

    operator bool() const noexcept
    {
        return found;
    }

    T const& operator*() const noexcept
    {
        return value;
    }
};

/*! parse tokens from a text.
 * This parsing tokens from most programming languages.
 * It will recognize:
 *    - integers literal
 *    - date literal
 *    - time literal
 *    - floating point literal
 *    - string literal
 *    - boolean literal
 *    - null
 *    - names
 *    - operators
 *    - comments (skip)
 *    - white space (skip)
 *
 * Errors will be returned as tokens which will point back into the text.
 */
[[nodiscard]] std::vector<token_t> parseTokens(std::string_view text) noexcept;

[[nodiscard]] std::vector<token_t>
parseTokens(std::string_view::const_iterator first, std::string_view::const_iterator last) noexcept;

} // namespace hi::inline v1

template<typename CharT>
auto std::formatter<hi::token_t, CharT>::format(hi::token_t const& t, auto& fc) const
    -> decltype(std::formatter<char const *, CharT>{}.format("", fc))
{
    return std::formatter<std::string_view, CharT>{}.format(t.repr(), fc);
}
