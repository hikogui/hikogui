// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/algorithm.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/assert.hpp"
#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/Unicode.hpp"
#include <string>
#include <string_view>
#include <iterator>
#include <vector>
#include <tuple>

namespace tt {

[[nodiscard]] constexpr bool isUpper(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

[[nodiscard]] constexpr bool isLower(char c) noexcept {
    return c >= 'a' && c <= 'z';
}

[[nodiscard]] constexpr bool isAlpha(char c) noexcept {
    return isUpper(c) || isLower(c);
}

[[nodiscard]] constexpr bool isDigit(char c) noexcept {
    return c >= '0' && c <= '9';
}

[[nodiscard]] constexpr bool isAlphaNum(char c) noexcept {
    return isAlpha(c) || isDigit(c);
}

[[nodiscard]] constexpr bool isLinefeed(char c) noexcept {
    return c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

[[nodiscard]] constexpr bool isWhitespace(char c) noexcept {
    return c == ' ' || c == '\t' || isLinefeed(c);
}

[[nodiscard]] constexpr bool isNumberFirst(char c) noexcept {
    return isDigit(c) || c == '+' || c == '-';
}

[[nodiscard]] constexpr bool isNameFirst(char c) noexcept {
    return isAlpha(c) || c == '_' || c == '$';
}

[[nodiscard]] constexpr bool isNameNext(char c) noexcept {
    return isAlphaNum(c) || c == '_' || c == '$';
}

[[nodiscard]] constexpr bool isQuote(char c) noexcept {
    return c == '"' || c == '\'' || c == '`';
}

[[nodiscard]] constexpr bool isOpenBracket(char c) noexcept {
    return c == '(' || c == '{' || c == '[';
}

[[nodiscard]] constexpr bool isCloseBracket(char c) noexcept {
    return c == ')' || c == '}' || c == ']';
}

[[nodiscard]] constexpr bool isOperator(char c) noexcept {
    return
        !isAlphaNum(c) && c != '_' &&
        !isWhitespace(c) &&
        !isQuote(c) &&
        !isOpenBracket(c) &&
        !isCloseBracket(c);
}

[[nodiscard]] inline std::string to_lower(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (let c: str) {
        r += (c >= 'A' && c <= 'Z') ? (c - 'A') + 'a': c;
    }

    return r;
}

[[nodiscard]] inline std::string to_upper(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (let c: str) {
        r += (c >= 'a' && c <= 'z') ? (c - 'a') + 'A': c;
    }

    return r;
}

/** Normalize string to use only line-feeds.
 */
[[nodiscard]] inline std::string normalize_lf(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    auto found_cr = false;
    for (let c: str) {
        if (ttauri_unlikely(found_cr)) {
            // This is Microsoft or old-Apple, we replace the previous carriage-return
            // with a line-feed and emit the current character.
            r += '\n';
            if (c != '\r' && c != '\n') {
                r += c;
            }
        
        } else if (ttauri_likely(c != '\r')) {
            // Emit any non-carriage return character.
            r += c;
        }

        found_cr = c == '\r';
    }
    if (found_cr) {
        r += '\n';
    }

    return r;
}

/** Encode a string to be usable as an id.
 * An id has the following format: [_a-zA-Z][_a-zA-Z0-9]*
 */
[[nodiscard]] inline std::string id_encode(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    r += isNameFirst(str.front()) ? str.front() : '_';
    for (let c: str.substr(1)) {
        r += isNameNext(c) ? c : '_';
    }

    return r;
}

gsl_suppress3(f.23,bounds.1,bounds.3)
[[nodiscard]] constexpr uint32_t fourcc(char const txt[5]) noexcept
{
    return (
        (static_cast<uint32_t>(txt[0]) << 24) |
        (static_cast<uint32_t>(txt[1]) << 16) |
        (static_cast<uint32_t>(txt[2]) <<  8) |
        static_cast<uint32_t>(txt[3])
        );
}

[[nodiscard]] constexpr uint32_t fourcc(uint8_t const *txt) noexcept
{
    return (
        (static_cast<uint32_t>(txt[0]) << 24) |
        (static_cast<uint32_t>(txt[1]) << 16) |
        (static_cast<uint32_t>(txt[2]) <<  8) |
        static_cast<uint32_t>(txt[3])
        );
}

gsl_suppress(bounds.3)
[[nodiscard]] inline std::string fourcc_to_string(uint32_t x) noexcept
{
    char c_str[5];
    c_str[0] = numeric_cast<char>((x >> 24) & 0xff);
    c_str[1] = numeric_cast<char>((x >> 16) & 0xff);
    c_str[2] = numeric_cast<char>((x >> 8) & 0xff);
    c_str[3] = numeric_cast<char>(x & 0xff);
    c_str[4] = 0;

    return {c_str};
}

[[nodiscard]] constexpr char nibble_to_char(uint8_t nibble) noexcept
{
    if (nibble <= 9) {
        return '0' + nibble;
    } else if (nibble <= 15) {
        return 'a' + nibble - 10;
    } else {
        no_default;
    }
}

/*!
 * \return value between 0-15, or -1 on error.
 */
[[nodiscard]] inline int8_t char_to_nibble(char c) noexcept
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return (c - 'a') + 10;
    } else if (c >= 'A' && c <= 'F') {
        return (c - 'A') + 10;
    } else {
        return -1;
    }
}

[[nodiscard]] inline std::string_view make_string_view(
    typename std::string::const_iterator b,
    typename std::string::const_iterator e
) noexcept
{
    return (b != e) ?
        std::string_view{&(*b), numeric_cast<size_t>(std::distance(b, e))} :
        std::string_view{};
}

template<typename Needle>
[[nodiscard]] size_t split_needle_size(Needle const &needle) noexcept
{
    return size(needle);
}

template<>
[[nodiscard]] inline size_t split_needle_size(char const &needle) noexcept
{
    return 1;
}

template<int N>
[[nodiscard]] inline size_t split_needle_size(char const (&needle)[N]) noexcept
{
    return N - 1;
}


template<typename Haystack, typename... Needles>
[[nodiscard]] auto split_find_needle(size_t offset, Haystack const &haystack, Needles const &... needles) noexcept
{
    return std::min(
        {std::pair{haystack.find(needles, offset), split_needle_size(needles)}...},
        [](let &a, let &b) {
            return a.first < b.first;
        }
    );
}

template<typename Haystack, typename... Needles>
[[nodiscard]] auto split(Haystack const &haystack, Needles const &... needles) noexcept
{
    std::vector<remove_cvref_t<Haystack>> r;

    size_t offset = 0;
    size_t needle_pos;
    size_t needle_size;

    std::tie(needle_pos, needle_size) = split_find_needle(offset, haystack, needles...);
    while (needle_pos != haystack.npos) {
        r.push_back(haystack.substr(offset, needle_pos - offset));

        offset = needle_pos + needle_size;
        std::tie(needle_pos, needle_size) = split_find_needle(offset, haystack, needles...);
    }

    r.push_back(haystack.substr(offset));
    return r;
}

[[nodiscard]] inline std::string join(std::vector<std::string> const &list, std::string_view const joiner = {}) noexcept
{
    std::string r;

    if (list.size() > 1) {
        size_t final_size = (list.size() - 1) * joiner.size();
        for (let &item: list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    int64_t i = 0;
    for (let &item: list) {
        if (i++ > 0) {
            r += joiner;
        }
        r += item;
    }
    return r;
}

[[nodiscard]] inline std::string join(std::vector<std::string_view> const &list, std::string_view const joiner = {}) noexcept
{
    std::string r;

    if (list.size() > 1) {
        size_t final_size = (list.size() - 1) * joiner.size();
        for (let &item: list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    int64_t i = 0;
    for (let &item: list) {
        if (i++ > 0) {
            r += joiner;
        }
        r += item;
    }
    return r;
}



/*! Return line and column count at the end iterator.
 */
template<typename It>
[[nodiscard]] inline std::pair<int,int> count_line_and_columns(It begin, It const end)
{
    int line = 1;
    int column = 1;

    for (; begin != end; begin++) {
        switch (*begin) {
        case '\n': line++; [[fallthrough]];
        case '\r': column = 1;
            break;
        case '\t':
            column = ((((column-1) / 8) + 1) * 8) + 1;
            break;
        default:
            column++;
        }
    }
    return { line, column };
}

}
