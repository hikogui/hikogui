// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "algorithm.hpp"
#include "cast.hpp"
#include "required.hpp"
#include "assert.hpp"
#include "os_detect.hpp"
#include "concepts.hpp"
#include "exception.hpp"
#include "codec/UTF.hpp"
#include <string>
#include <string_view>
#include <iterator>
#include <vector>
#include <tuple>
#include <type_traits>

namespace tt {

[[nodiscard]] constexpr bool isUpper(char c) noexcept
{
    return c >= 'A' && c <= 'Z';
}

[[nodiscard]] constexpr bool isLower(char c) noexcept
{
    return c >= 'a' && c <= 'z';
}

[[nodiscard]] constexpr bool isAlpha(char c) noexcept
{
    return isUpper(c) || isLower(c);
}

[[nodiscard]] constexpr bool isDigit(char c) noexcept
{
    return c >= '0' && c <= '9';
}

[[nodiscard]] constexpr bool isAlphaNum(char c) noexcept
{
    return isAlpha(c) || isDigit(c);
}

[[nodiscard]] constexpr bool isLinefeed(char c) noexcept
{
    return c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

[[nodiscard]] constexpr bool isWhitespace(char c) noexcept
{
    return c == ' ' || c == '\t' || isLinefeed(c);
}

[[nodiscard]] constexpr bool isNumberFirst(char c) noexcept
{
    return isDigit(c) || c == '+' || c == '-';
}

[[nodiscard]] constexpr bool isNameFirst(char c) noexcept
{
    return isAlpha(c) || c == '_' || c == '$';
}

[[nodiscard]] constexpr bool isNameNext(char c) noexcept
{
    return isAlphaNum(c) || c == '_' || c == '$';
}

[[nodiscard]] constexpr bool isQuote(char c) noexcept
{
    return c == '"' || c == '\'' || c == '`';
}

[[nodiscard]] constexpr bool isOpenBracket(char c) noexcept
{
    return c == '(' || c == '{' || c == '[';
}

[[nodiscard]] constexpr bool isCloseBracket(char c) noexcept
{
    return c == ')' || c == '}' || c == ']';
}

[[nodiscard]] constexpr bool isOperator(char c) noexcept
{
    return !isAlphaNum(c) && c != '_' && !isWhitespace(c) && !isQuote(c) && !isOpenBracket(c) && !isCloseBracket(c);
}

[[nodiscard]] inline std::string to_lower(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (ttlet c : str) {
        r += (c >= 'A' && c <= 'Z') ? (c - 'A') + 'a' : c;
    }

    return r;
}

[[nodiscard]] inline std::string to_upper(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (ttlet c : str) {
        r += (c >= 'a' && c <= 'z') ? (c - 'a') + 'A' : c;
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
    for (ttlet c : str) {
        if (found_cr) {
            // This is Microsoft or old-Apple, we replace the previous carriage-return
            // with a line-feed and emit the current character.
            [[unlikely]] r += '\n';
            if (c != '\r' && c != '\n') {
                r += c;
            }

        } else if (c != '\r') {
            // Emit any non-carriage return character.
            [[likely]] r += c;
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
    for (ttlet c : str.substr(1)) {
        r += isNameNext(c) ? c : '_';
    }

    return r;
}

[[nodiscard]] constexpr uint32_t fourcc(char const txt[5]) noexcept
{
    return (
        (static_cast<uint32_t>(txt[0]) << 24) | (static_cast<uint32_t>(txt[1]) << 16) | (static_cast<uint32_t>(txt[2]) << 8) |
        static_cast<uint32_t>(txt[3]));
}

[[nodiscard]] constexpr uint32_t fourcc(uint8_t const *txt) noexcept
{
    return (
        (static_cast<uint32_t>(txt[0]) << 24) | (static_cast<uint32_t>(txt[1]) << 16) | (static_cast<uint32_t>(txt[2]) << 8) |
        static_cast<uint32_t>(txt[3]));
}

[[nodiscard]] inline std::string fourcc_to_string(uint32_t x) noexcept
{
    char c_str[5];
    c_str[0] = narrow_cast<char>((x >> 24) & 0xff);
    c_str[1] = narrow_cast<char>((x >> 16) & 0xff);
    c_str[2] = narrow_cast<char>((x >> 8) & 0xff);
    c_str[3] = narrow_cast<char>(x & 0xff);
    c_str[4] = 0;

    return {c_str};
}

constexpr size_t string_size(sizeable auto str) noexcept
{
    return std::size(str);
}

constexpr size_t string_size(auto str) noexcept
{
    return 1;
}

template<typename FirstNeedle, typename... Needles>
[[nodiscard]] std::pair<size_t, size_t>
    string_find_any(std::string_view haystack, size_t pos, FirstNeedle const &first_needle, Needles const &...needles) noexcept
{
    using std::size;

    size_t first = haystack.find(first_needle, pos);
    size_t last = first + string_size(first_needle);

    if (first == std::string_view::npos) {
        first = size(haystack);
        last = size(haystack);
    }

    if constexpr (sizeof...(Needles) != 0) {
        ttlet [other_first, other_last] = string_find_any(haystack, pos, needles...);
        if (other_first < first) {
            first = other_first;
            last = other_last;
        }
    }

    return {first, last};
}

template<typename StringType, typename... Needles>
[[nodiscard]] std::vector<StringType> _split(std::string_view haystack, Needles const &...needles) noexcept
{
    auto r = std::vector<StringType>{};

    std::string_view::size_type current_pos = 0;

    while (current_pos < std::size(haystack)) {
        ttlet [needle_first, needle_last] = string_find_any(haystack, current_pos, needles...);
        r.push_back(StringType{haystack.substr(current_pos, needle_first - current_pos)});
        current_pos = needle_last;
    }

    return r;
}

template<typename... Needles>
[[nodiscard]] std::vector<std::string> split(std::string_view haystack, Needles const &...needles) noexcept
{
    return _split<std::string>(haystack, needles...);
}

[[nodiscard]] inline std::vector<std::string> split(std::string_view haystack) noexcept
{
    return split(haystack, ' ');
}

template<typename... Needles>
[[nodiscard]] std::vector<std::string_view> split_view(std::string_view haystack, Needles const &...needles) noexcept
{
    return _split<std::string_view>(haystack, needles...);
}

[[nodiscard]] inline std::vector<std::string_view> split_view(std::string_view haystack) noexcept
{
    return split_view(haystack, ' ');
}

template<typename CharT>
[[nodiscard]] std::basic_string<CharT>
join(std::vector<std::basic_string<CharT>> const &list, std::basic_string_view<CharT> const joiner = {}) noexcept
{
    std::string r;

    if (list.size() > 1) {
        size_t final_size = (list.size() - 1) * joiner.size();
        for (ttlet &item : list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    size_t i = 0;
    for (ttlet &item : list) {
        if (i++ != 0) {
            r += joiner;
        }
        r += item;
    }
    return r;
}

template<typename CharT>
[[nodiscard]] std::basic_string<CharT>
join(std::vector<std::basic_string<CharT>> const &list, std::basic_string<CharT> const &joiner) noexcept
{
    return join(list, std::basic_string_view<CharT>{joiner});
}

template<typename CharT>
[[nodiscard]] std::basic_string<CharT> join(std::vector<std::basic_string<CharT>> const &list, CharT const *joiner) noexcept
{
    return join(list, std::basic_string_view<CharT>{joiner});
}

[[nodiscard]] inline std::string join(std::vector<std::string_view> const &list, std::string_view const joiner = {}) noexcept
{
    std::string r;

    if (list.size() > 1) {
        size_t final_size = (list.size() - 1) * joiner.size();
        for (ttlet &item : list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    int64_t i = 0;
    for (ttlet &item : list) {
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
[[nodiscard]] inline std::pair<int, int> count_line_and_columns(It begin, It const end)
{
    int line = 1;
    int column = 1;

    for (; begin != end; begin++) {
        switch (*begin) {
        case '\n': line++; [[fallthrough]];
        case '\r': column = 1; break;
        case '\t': column = ((((column - 1) / 8) + 1) * 8) + 1; break;
        default: column++;
        }
    }
    return {line, column};
}

/** Create an std::array from a one dimensional array, without the last element.
 * Useful for copying a string literal without the nul-termination
 */
template<typename T, size_t N>
constexpr auto to_array_without_last(T (&rhs)[N]) noexcept
{
    auto r = std::array<std::remove_cv_t<T>, N - 1>{};
    for (size_t i = 0; i != (N - 1); ++i) {
        r[i] = rhs[i];
    }
    return r;
}

/** Create an std::array from a one dimensional array, without the last element.
 * Useful for copying a string literal without the nul-termination
 */
template<typename T, size_t N>
constexpr auto to_array_without_last(T(&&rhs)[N]) noexcept
{
    auto r = std::array<std::remove_cv_t<T>, N - 1>{};
    for (size_t i = 0; i != (N - 1); ++i) {
        r[i] = std::move(rhs[i]);
    }
    return r;
}

[[nodiscard]] inline std::string lstrip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto first = front_strip(std::begin(haystack), std::end(haystack), std::begin(needle), std::end(needle));
    return std::string{first, std::end(haystack)};
}

[[nodiscard]] inline std::string rstrip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto last = back_strip(std::begin(haystack), std::end(haystack), std::begin(needle), std::end(needle));
    return std::string{std::begin(haystack), last};
}

[[nodiscard]] inline std::string strip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto first = front_strip(std::begin(haystack), std::end(haystack), std::begin(needle), std::end(needle));
    auto last = back_strip(first, std::end(haystack), std::begin(needle), std::end(needle));
    return std::string{first, last};
}

/** Convert a win32 zero terminated list of zero terminated strings.
 * @param first A pointer to a buffer of a zero terminated list of zero terminated string.
 * @param last A pointer one beyond the buffer.
 * @param nr_strings The number of string in the buffer.
 * @return A vector of UTF-8 encoded strings.
 * @throws parse_error when the list does not terminate with a zero.
 */
[[nodiscard]] inline std::vector<std::string> ZZWSTR_to_string(wchar_t *first, wchar_t *last, ssize_t nr_strings=-1)
{
    auto r = std::vector<std::string>{};

    while (first != last) {
        auto it_zero = std::find(first, last, wchar_t{0});
        if (it_zero == last) {
            throw parse_error("Could not find terminating zero of a string.");
        }

        auto ws = std::wstring_view{first, narrow_cast<size_t>(it_zero - first)};
        if (ws.empty()) {
            // The list is terminated with an empty string.
            break;
        }

        r.push_back(tt::to_string(ws));

        // Continue after the zero terminator.
        first = it_zero + 1;
    }

    if (nr_strings != -1 && std::ssize(r) != nr_strings) {
        throw parse_error("Unexpected number of string in list.");
    }

    return r;
}

} // namespace tt
