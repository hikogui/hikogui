// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/strings.hpp"
#include "ttauri/foundation/small_vector.hpp"
#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/decimal.hpp"
#include "ttauri/foundation/exceptions.hpp"
#include "ttauri/foundation/parse_location.hpp"
#include <date/date.h>
#include <memory>
#include <string>
#include <string_view>
#include <charconv>
#include <array>

namespace tt {

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
    Operator,                // Operator, or bracket, or other literal text.
    End
};

constexpr char const *to_string(tokenizer_name_t name) noexcept {
    switch (name) {
    case tokenizer_name_t::NotAssigned: return "NotAssigned";
    case tokenizer_name_t::ErrorInvalidCharacter: return "ErrorInvalidCharacter";
    case tokenizer_name_t::ErrorEOTInBlockComment: return "ErrorEOTInBlockComment";
    case tokenizer_name_t::ErrorEOTInString: return "ErrorEOTInString";
    case tokenizer_name_t::ErrorLFInString: return "ErrorLFInString";
    case tokenizer_name_t::Name: return "Name";
    case tokenizer_name_t::StringLiteral: return "StringLiteral";
    case tokenizer_name_t::IntegerLiteral: return "IntegerLiteral";
    case tokenizer_name_t::DateLiteral: return "DateLiteral";
    case tokenizer_name_t::TimeLiteral: return "TimeLiteral";
    case tokenizer_name_t::FloatLiteral: return "FloatLiteral";
    case tokenizer_name_t::Operator: return "Operator";
    case tokenizer_name_t::End: return "End";
    default: tt_no_default;
    }
}

inline std::ostream &operator<<(std::ostream &lhs, tokenizer_name_t rhs)
{
    return lhs << to_string(rhs);
}

struct token_t {
    tokenizer_name_t name;
    std::string value;
    parse_location location;
    bool is_binary;
    int precedence;

    token_t() noexcept :
        name(tokenizer_name_t::NotAssigned), value(), location(), is_binary(false), precedence(0) {}

    token_t(tokenizer_name_t name, std::string value) noexcept :
        name(name), value(std::move(value)), location(), is_binary(false), precedence(0) {}

    token_t(token_t const &other) noexcept :
        name(other.name), value(other.value), location(other.location), is_binary(other.is_binary), precedence(other.precedence) {}

    token_t(token_t &&other) noexcept :
        name(other.name), value(std::move(other.value)), location(std::move(other.location)), is_binary(other.is_binary), precedence(other.precedence) {}

    token_t &operator=(token_t const &other) noexcept {
        if (this != &other) {
            name = other.name;
            value = other.value;
            location = other.location;
            is_binary = other.is_binary;
            precedence = other.precedence;
        }
        return *this;
    }

    token_t &operator=(token_t &&other) noexcept {
        using std::move;
        name = move(other.name);
        value = move(other.value);
        location = move(other.location);
        is_binary = move(other.is_binary);
        precedence = move(other.precedence);
        return *this;
    }

    operator bool () const noexcept {
        return name != tokenizer_name_t::NotAssigned;
    }

    explicit operator long double () const {
        try {
            return std::stold(value);
        } catch(...) {
            TTAURI_THROW(parse_error("Could not convert token {} to long double", *this));
        }
    }

    explicit operator double () const {
        try {
            return std::stod(value);
        } catch(...) {
            TTAURI_THROW(parse_error("Could not convert token {} to double", *this));
        }
    }

    explicit operator float () const {
        try {
            return std::stof(value);
        } catch(...) {
            TTAURI_THROW(parse_error("Could not convert token {} to float", *this));
        }
    }

    explicit operator signed long long () const {
        try {
            return std::stoll(value);
        } catch(...) {
            TTAURI_THROW(parse_error("Could not convert token {} to signed long long", *this));
        }
    }

    explicit operator signed long () const {
        auto v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed long>::min() || v > std::numeric_limits<signed long>::max()) {
            TTAURI_THROW(parse_error("Could not convert token {} to signed long", *this))
        }
        return static_cast<signed long>(v);
    }

    explicit operator signed int () const {
        auto v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed int>::min() || v > std::numeric_limits<signed int>::max()) {
            TTAURI_THROW(parse_error("Could not convert token {} to signed int", *this))
        }
        return static_cast<signed int>(v);
    }

    explicit operator signed short () const {
        auto v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed short>::min() || v > std::numeric_limits<signed short>::max()) {
            TTAURI_THROW(parse_error("Could not convert token {} to signed short", *this))
        }
        return static_cast<signed short>(v);
    }

    explicit operator signed char () const {
        auto v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed char>::min() || v > std::numeric_limits<signed char>::max()) {
            TTAURI_THROW(parse_error("Could not convert token {} to signed char", *this))
        }
        return static_cast<signed char>(v);
    }

    explicit operator unsigned long long () const {
        auto v = static_cast<signed long long>(*this);
        if (v < 0) {
            TTAURI_THROW(parse_error("Could not convert token {} to unsigned long long", *this))
        }
        return static_cast<unsigned long long>(v);
    }

    explicit operator unsigned long () const {
        auto v = static_cast<signed long long>(*this);
        if (v < 0 || v > std::numeric_limits<unsigned long>::max()) {
            TTAURI_THROW(parse_error("Could not convert token {} to unsigned long", *this))
        }
        return static_cast<unsigned long>(v);
    }

    explicit operator unsigned int () const {
        auto v = static_cast<signed long long>(*this);
        if (v < 0 || v > std::numeric_limits<unsigned int>::max()) {
            TTAURI_THROW(parse_error("Could not convert token {} to unsigned int", *this))
        }
        return static_cast<unsigned int>(v);
    }

    explicit operator unsigned short () const {
        auto v = static_cast<signed long long>(*this);
        if (v < 0 || v > std::numeric_limits<unsigned short>::max()) {
            TTAURI_THROW(parse_error("Could not convert token {} to unsigned short", *this))
        }
        return static_cast<unsigned short>(v);
    }

    explicit operator unsigned char () const {
        auto v = static_cast<signed long long>(*this);
        if (v < 0 || v > std::numeric_limits<unsigned char>::max()) {
            TTAURI_THROW(parse_error("Could not convert token {} to unsigned char", *this))
        }
        return static_cast<unsigned char>(v);
    }

    explicit operator std::string () const noexcept {
        return value;
    }

    explicit operator decimal () const {
        return decimal{value};
    }

    explicit operator date::year_month_day () const {
        ttlet parts = split(value, "-");
        if (parts.size() != 3) {
            TTAURI_THROW(parse_error("Expect date to be in the format YYYY-MM-DD"));
        }

        ttlet year = date::year{stoi(parts[0])};
        ttlet month = date::month{numeric_cast<unsigned int>(stoi(parts[1]))};
        ttlet day = date::day{numeric_cast<unsigned int>(stoi(parts[2]))};
        return {year, month, day};
    }

    std::string repr() const noexcept {
        std::string r = to_string(name);
        if (value.size() > 0) {
            r += '\"';
            r += value;
            r += '\"';
        }
        return r;
    }

    friend inline std::ostream &operator<<(std::ostream &lhs, token_t const &rhs)
    {
        return lhs << rhs.repr();
    }

    [[nodiscard]] friend tt_force_inline bool operator==(token_t const &lhs, token_t const &rhs) noexcept {
        return (lhs.name == rhs.name) && (lhs.value == rhs.value);
    }

    [[nodiscard]] friend tt_force_inline bool operator==(token_t const &lhs, tokenizer_name_t const &rhs) noexcept {
        return lhs.name == rhs;
    }

    [[nodiscard]] friend tt_force_inline bool operator!=(token_t const &lhs, tokenizer_name_t const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend tt_force_inline bool operator==(token_t const &lhs, const char *rhs) noexcept {
        return lhs.value == rhs;
    }

    [[nodiscard]] friend tt_force_inline bool operator!=(token_t const &lhs, const char *rhs) noexcept {
        return !(lhs == rhs);
    }

};

using token_vector = std::vector<tt::token_t>;
using token_iterator = typename token_vector::iterator;

template<typename T>
struct parse_result {
    bool found;
    T value;
    token_iterator next_token;

    parse_result() noexcept :
        found(false), value(), next_token() {}

    parse_result(T const &value, token_iterator next_token) :
        found(true), value(value), next_token(next_token) {}

    operator bool () const noexcept {
        return found;
    }

    T const &operator*() const noexcept {
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

[[nodiscard]] std::vector<token_t> parseTokens(std::string_view::const_iterator first, std::string_view::const_iterator last) noexcept;



}

