

#pragma once

#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/small_vector.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/fixed.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <charconv>
#include <array>

namespace TTauri {

enum class tokenizer_name_t : uint8_t {
    NotAssigned,
    ErrorInvalidCharacter,
    ErrorEOTInBlockComment,
    ErrorEOTInString,
    ErrorLFInString,

    Operator,
    Name,
    StringLiteral,
    IntegerLiteral,
    DateLiteral,
    TimeLiteral,
    FloatLiteral,
    Literal,                // Operator, or bracket, or other literal text.
    End
};

constexpr char const *to_string(tokenizer_name_t name) noexcept {
    switch (name) {
    case tokenizer_name_t::NotAssigned: return "NotAssigned";
    case tokenizer_name_t::ErrorInvalidCharacter: return "ErrorInvalidCharacter";
    case tokenizer_name_t::ErrorEOTInBlockComment: return "ErrorEOTInBlockComment";
    case tokenizer_name_t::ErrorEOTInString: return "ErrorEOTInString";
    case tokenizer_name_t::ErrorLFInString: return "ErrorLFInString";
    case tokenizer_name_t::Operator: return "Operator";
    case tokenizer_name_t::Name: return "Name";
    case tokenizer_name_t::StringLiteral: return "StringLiteral";
    case tokenizer_name_t::IntegerLiteral: return "IntegerLiteral";
    case tokenizer_name_t::DateLiteral: return "DateLiteral";
    case tokenizer_name_t::TimeLiteral: return "TimeLiteral";
    case tokenizer_name_t::FloatLiteral: return "FloatLiteral";
    case tokenizer_name_t::Literal: return "Literal";
    case tokenizer_name_t::End: return "End";
    default: no_default;
    }
}

inline std::ostream &operator<<(std::ostream &lhs, tokenizer_name_t rhs)
{
    return lhs << to_string(rhs);
}

struct token_t {
    tokenizer_name_t name = tokenizer_name_t::NotAssigned;
    std::string value;
    std::string_view::iterator index;

    operator bool () const noexcept {
        return name != tokenizer_name_t::NotAssigned;
    }

    explicit operator int () const noexcept {
        return std::stoi(value);
    }

    explicit operator std::string () const noexcept {
        return value;
    }

    template<typename T, int M>
    explicit operator fixed<T,M> () const {
        return fixed<T,M>{value};
    }

    explicit operator date::year_month_day () const {
        let parts = split(value, "-");
        if (parts.size() != 3) {
            TTAURI_THROW(parse_error("Expect date to be in the format YYYY-MM-DD"));
        }

        let year = date::year{stoi(parts[0])};
        let month = date::month{numeric_cast<unsigned int>(stoi(parts[1]))};
        let day = date::day{numeric_cast<unsigned int>(stoi(parts[2]))};
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
};

using token_vector = std::vector<TTauri::token_t>;
using token_iterator = typename token_vector::iterator;

template<typename T>
struct parse_result_t {
    std::optional<T> value;
    token_iterator next_token;

    operator bool () const noexcept {
        return static_cast<bool>(value);
    }
};

inline std::ostream &operator<<(std::ostream &lhs, token_t const &rhs)
{
    return lhs << rhs.repr();
}

force_inline bool operator==(token_t const &lhs, token_t const &rhs) noexcept {
    return (lhs.name == rhs.name) && (lhs.value == rhs.value) && (lhs.index == rhs.index);
}

force_inline bool operator==(token_t const &lhs, tokenizer_name_t const &rhs) noexcept {
    return lhs.name == rhs;
}

force_inline bool operator!=(token_t const &lhs, tokenizer_name_t const &rhs) noexcept {
    return !(lhs == rhs);
}

force_inline bool operator==(token_t const &lhs, const char *rhs) noexcept {
    return lhs.value == rhs;
}

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



}

