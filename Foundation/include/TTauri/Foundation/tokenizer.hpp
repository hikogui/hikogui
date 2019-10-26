

#pragma once

#include "TTauri/Foundation/datum.hpp"
#include "TTauri/Foundation/required.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <charconv>

namespace TTauri {

enum class TokenName {
    Whitespace,
    Comment,
    Operator,
    Name,
    StringLiteral,
    IntegerLiteral,
    FloatLiteral,
    BooleanLiteral,
    Open,
    Close,
    Error
};

struct Token {
    TokenName name;
    datum value;
    size_t offset;
};

inline bool operator==(Token const &lhs, Token const &rhs) noexcept
{
    return (lhs.name == rhs.name) && (lhs.value == rhs.value) && (lhs.offset == rhs.offset);
}

force_inline bool isUpper(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

force_inline bool isLower(char c) noexcept {
    return c >= 'a' && c <= 'z';
}

force_inline bool isAlpha(char c) noexcept {
    return isUpper(c) || isLower(c);
}

force_inline bool isDigit(char c) noexcept {
    return c >= '0' && c <= '9';
}

force_inline bool isAlphaNum(char c) noexcept {
    return isAlpha(c) || isDigit(c);
}

force_inline bool isLinefeed(char c) noexcept {
    return c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

force_inline bool isWhitespace(char c) noexcept {
    return c == ' ' || c == '\t' || isLinefeed(c);
}

force_inline bool isNumberFirst(char c) noexcept {
    return isDigit(c) || c == '+' || c == '-';
}

force_inline bool isNameFirst(char c) noexcept {
    return isAlpha(c) || c == '_';
}

force_inline bool isNameNext(char c) noexcept {
    return isAlphaNum(c) || c == '_';
}

force_inline bool isQuote(char c) noexcept {
    return c == '"' || c == '\'' || c == '`';
}

force_inline bool isOpenBracket(char c) noexcept {
    return c == '(' || c == '{' || c == '[';
}

force_inline bool isCloseBracket(char c) noexcept {
    return c == ')' || c == '}' || c == ']';
}

force_inline bool isOperator(char c) noexcept {
    return
        !isAlphaNum(c) && c != '_' &&
        !isWhitespace(c) &&
        !isQuote(c) &&
        !isOpenBracket(c) &&
        !isCloseBracket(c);
}

inline Token parseNameToken(std::string_view text, size_t &offset)
{
    let start = offset;
    offset++;

    for (; offset < text.size(); offset++) {
        let c = text[offset];
        if (!isNameNext(c)) {
            return {TokenName::Name, datum{text.substr(start, offset - start)}, start};
        }
    }
    return {TokenName::Name, datum{text.substr(start, offset - start)}, start};
}

inline Token parseOperatorToken(std::string_view text, size_t &offset)
{
    let start = offset;
    offset++;

    for (; offset < text.size(); offset++) {
        let c = text[offset];
        if (!isOperator(c)) {
            return {TokenName::Operator, datum{text.substr(start, offset - start)}, start};
        }
    }
    return {TokenName::Operator, datum{text.substr(start, offset - start)}, start};
}

inline Token parseNumberToken(std::string_view text, size_t &offset)
{
    if (text[offset] == '+' || text[offset] == '-') {
        if (offset + 1 >= text.size() || text[offset+1] <= '0' || text[offset+1] >= '9') {
            return parseOperatorToken(text, offset);
        }
    }

    let start = offset;

    double doubleValue;
    int64_t intValue;

    char const *beginPtr = &text[offset];
    char const *endPtr = &text[text.size()-1] + 1;

    let doubleResult = std::from_chars(beginPtr, endPtr, doubleValue);
    let intResult = std::from_chars(beginPtr, endPtr, intValue);
    if (doubleResult.ptr > intResult.ptr) {
        let size = doubleResult.ptr - beginPtr;
        offset += size;
        return {TokenName::FloatLiteral, datum{doubleValue}, start};
    } else {
        let size = intResult.ptr - beginPtr;
        offset += size;
        return {TokenName::IntegerLiteral, datum{intValue}, start};
    }
}

inline Token parseStringToken(std::string_view text, size_t &offset)
{
    std::string r;

    let start = offset;
    let quote = text[offset++];
    r += quote;

    bool escape = false;
    for (; offset < text.size(); offset++) {
        let c = text[offset];
        if (c == '\\') {
            escape = true;

        } else if (escape) {
            switch (c) {
            case 'a': r += '\a'; break;
            case 'b': r += '\b'; break;
            case 'f': r += '\f'; break;
            case 'n': r += '\n'; break;
            case 'r': r += '\r'; break;
            case 't': r += '\t'; break;
            case 'v': r += '\v'; break;
            default: r += c;
            }
            escape = false;
        } else if (c == quote) {
            offset++;
            return {TokenName::StringLiteral, datum{r}, start};

        } else {
            r += c;
        }
    }
    return {TokenName::Error, datum{}, start};
}

inline Token parseWhitespaceToken(std::string_view text, size_t &offset)
{
    let start = offset;
    offset++;

    for (; offset < text.size(); offset++) {
        let c = text[offset];
        if (!isWhitespace(c)) {
            return {TokenName::Whitespace, datum{}, start};
        }
    }
    return {TokenName::Whitespace, datum{}, start};
}

inline Token parseLineCommentToken(std::string_view text, size_t &offset)
{
    let start = offset;
    offset++;

    for (; offset < text.size(); offset++) {
        let c = text[offset];
        if (isLinefeed(c)) {
            return {TokenName::Comment, datum{}, start};
        }
    }
    return {TokenName::Comment, datum{}, start};
}

inline Token parseBlockCommentToken(std::string_view text, size_t &offset)
{
    let start = offset;
    offset += 2;

    while (offset < text.size()) {
        let c = text[offset++];
        let nextC = (offset < text.size()) ? text[offset] : '\0';
        if (c == '*' && nextC == '/') {
            offset++;
            return {TokenName::Comment, datum{}, start};
        }
    }
    return {TokenName::Error, datum{}, start};
}

inline Token parseCommentToken(std::string_view text, size_t &offset)
{
    let c = text[offset];
    if (c == '#') {
        return parseLineCommentToken(text, offset);
    } else {
        axiom_assert(c == '/');

        let nextC = (offset + 1 < text.size()) ? text[offset+1] : '\0';
        if (nextC == '/') {
            return parseLineCommentToken(text, offset);
        } else if (nextC == '*') {
            return parseBlockCommentToken(text, offset);
        } else {
            return parseOperatorToken(text, offset);
        }
    }
}


/*! Parse a token.
 * This tokenizer is for parsing tokens in most languages.
 * It will recognize:
 *    - integers literal
 *    - floating point literal
 *    - string literal
 *    - boolean literal
 *    - null
 *    - names
 *    - operators
 *    - comments
 *    - white space
 */
Token parseToken(std::string_view text, size_t &offset)
{
    let c = text[offset];
    if (isNameFirst(c)) {
        return parseNameToken(text, offset);
    } else if (isNumberFirst(c)) {
        return parseNumberToken(text, offset);
    } else if (isQuote(c)) {
        return parseStringToken(text, offset);
    } else if (isWhitespace(c)) {
        return parseWhitespaceToken(text, offset);
    } else if (isOpenBracket(c)) {
        return {TokenName::Open, datum{std::string(1, c)}, offset++};
    } else if (isCloseBracket(c)) {
        return {TokenName::Close, datum{std::string(1, c)}, offset++};
    } else if (c == '#' || c == '/') {
        return parseCommentToken(text, offset);
    } else {
        return parseOperatorToken(text, offset);
    }
}


}

