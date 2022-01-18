// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "algorithm.hpp"
#include "cast.hpp"
#include "required.hpp"
#include "assert.hpp"
#include "architecture.hpp"
#include "concepts.hpp"
#include "exception.hpp"
#include "unicode/UTF.hpp"
#include <string>
#include <string_view>
#include <iterator>
#include <vector>
#include <tuple>
#include <type_traits>
#include <cstdlib>

namespace tt::inline v1 {

[[nodiscard]] constexpr bool is_upper(char c) noexcept
{
    return c >= 'A' && c <= 'Z';
}

[[nodiscard]] constexpr bool is_lower(char c) noexcept
{
    return c >= 'a' && c <= 'z';
}

[[nodiscard]] constexpr bool is_alpha(char c) noexcept
{
    return is_upper(c) || is_lower(c);
}

[[nodiscard]] constexpr bool is_digit(char c) noexcept
{
    return c >= '0' && c <= '9';
}

[[nodiscard]] constexpr bool is_alpha_num(char c) noexcept
{
    return is_alpha(c) || is_digit(c);
}

[[nodiscard]] constexpr bool is_line_feed(char c) noexcept
{
    return c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

[[nodiscard]] constexpr bool is_white_space(char c) noexcept
{
    return c == ' ' || c == '\t' || is_line_feed(c);
}

[[nodiscard]] constexpr bool is_number_first(char c) noexcept
{
    return is_digit(c) || c == '+' || c == '-';
}

[[nodiscard]] constexpr bool is_name_first(char c) noexcept
{
    return is_alpha(c) or c == '_' or c == '$' or (c & 0x80);
}

[[nodiscard]] constexpr bool is_name_next(char c) noexcept
{
    return is_alpha_num(c) or c == '_' or c == '$' or (c & 0x80);
}

[[nodiscard]] constexpr bool is_quote(char c) noexcept
{
    return c == '"' || c == '\'' || c == '`';
}

[[nodiscard]] constexpr bool is_open_bracket(char c) noexcept
{
    return c == '(' || c == '{' || c == '[';
}

[[nodiscard]] constexpr bool is_close_bracket(char c) noexcept
{
    return c == ')' || c == '}' || c == ']';
}

[[nodiscard]] constexpr bool is_operator(char c) noexcept
{
    return !is_alpha_num(c) && c != '_' && !is_white_space(c) && !is_quote(c) && !is_open_bracket(c) && !is_close_bracket(c);
}

[[nodiscard]] constexpr char to_lower(char c) noexcept
{
    return (c >= 'A' and c <= 'Z') ? (c - 'A') + 'a' : c;
}

[[nodiscard]] constexpr char to_upper(char c) noexcept
{
    return (c >= 'a' and c <= 'z') ? (c - 'a') + 'A' : c;
}

[[nodiscard]] inline std::string to_lower(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (ttlet c : str) {
        r += to_lower(c);
    }

    return r;
}

[[nodiscard]] inline std::string to_upper(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (ttlet c : str) {
        r += to_upper(c);
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
[[nodiscard]] inline std::string make_identifier(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    r += is_name_first(str.front()) ? str.front() : '_';
    for (ttlet c : str.substr(1)) {
        r += is_name_next(c) ? c : '_';
    }

    return r;
}

/** Create a slug from a string.
 * A slug contains only lower case letters, digits and dashes.
 * Duplicated dashes are eliminated.
 */
[[nodiscard]] inline std::string make_slug(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    std::size_t dash_count = 0;
    for (ttlet c : str) {
        if (is_alpha_num(c)) {
            dash_count = 0;
            r += to_lower(c);
        } else if (dash_count++ == 0) {
            r += '-';
        }
    }

    return r;
}

/** Create a title from a string.
 * A title contains words separated by a single space, where each word starts with
 * a capital letter followed by lower case letters. Digits may be part of a word
 * or form a separate word, digits are not counted as the start of a word for
 * capitalization.
 */
[[nodiscard]] inline std::string make_title(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    // Do not start with a space.
    std::size_t space_count = 1;
    std::size_t letter_count = 0;
    for (ttlet c : str) {
        if (is_alpha_num(c)) {
            if (is_digit(c)) {
                r += c;
            } else if (letter_count++ == 0) {
                r += to_upper(c);
            } else {
                r += to_lower(c);
            }
            space_count = 0;

        } else if (space_count++ == 0) {
            r += ' ';
            letter_count = 0;
        }
    }

    if (space_count) {
        // Strip trailing space.
        r.resize(r.size() - 1);
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
    c_str[0] = static_cast<char>((x >> 24) & 0xff);
    c_str[1] = static_cast<char>((x >> 16) & 0xff);
    c_str[2] = static_cast<char>((x >> 8) & 0xff);
    c_str[3] = static_cast<char>(x & 0xff);
    c_str[4] = 0;

    return {c_str};
}

constexpr std::size_t string_size(sizeable auto str) noexcept
{
    return size(str);
}

constexpr std::size_t string_size(auto str) noexcept
{
    return 1;
}

template<typename FirstNeedle, typename... Needles>
[[nodiscard]] std::pair<std::size_t, std::size_t>
string_find_any(std::string_view haystack, std::size_t pos, FirstNeedle const &first_needle, Needles const &...needles) noexcept
{
    using std::size;

    std::size_t first = haystack.find(first_needle, pos);
    std::size_t last = first + string_size(first_needle);

    if (first == std::string_view::npos) {
        first = size(haystack);
        last = size(haystack);
    }

    if constexpr (sizeof...(Needles) != 0) {
        ttlet[other_first, other_last] = string_find_any(haystack, pos, needles...);
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

    while (current_pos < size(haystack)) {
        ttlet[needle_first, needle_last] = string_find_any(haystack, current_pos, needles...);
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
        std::size_t final_size = (list.size() - 1) * joiner.size();
        for (ttlet &item : list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    std::size_t i = 0;
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
        std::size_t final_size = (list.size() - 1) * joiner.size();
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
template<typename T, std::size_t N>
constexpr auto to_array_without_last(T (&rhs)[N]) noexcept
{
    auto r = std::array<std::remove_cv_t<T>, N - 1>{};
    for (std::size_t i = 0; i != (N - 1); ++i) {
        r[i] = rhs[i];
    }
    return r;
}

/** Create an std::array from a one dimensional array, without the last element.
 * Useful for copying a string literal without the nul-termination
 */
template<typename T, std::size_t N>
constexpr auto to_array_without_last(T(&&rhs)[N]) noexcept
{
    auto r = std::array<std::remove_cv_t<T>, N - 1>{};
    for (std::size_t i = 0; i != (N - 1); ++i) {
        r[i] = std::move(rhs[i]);
    }
    return r;
}

[[nodiscard]] inline std::string lstrip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto first = front_strip(begin(haystack), end(haystack), begin(needle), end(needle));
    return std::string{first, end(haystack)};
}

[[nodiscard]] inline std::string rstrip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto last = back_strip(begin(haystack), end(haystack), begin(needle), end(needle));
    return std::string{begin(haystack), last};
}

[[nodiscard]] inline std::string strip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto first = front_strip(begin(haystack), end(haystack), begin(needle), end(needle));
    auto last = back_strip(first, end(haystack), begin(needle), end(needle));
    return std::string{first, last};
}

/** Convert a win32 zero terminated list of zero terminated strings.
 * @param first A pointer to a buffer of a zero terminated list of zero terminated string.
 * @param last A pointer one beyond the buffer.
 * @param nr_strings The number of string in the buffer.
 * @return A vector of UTF-8 encoded strings.
 * @throws parse_error when the list does not terminate with a zero.
 */
[[nodiscard]] inline std::vector<std::string> ZZWSTR_to_string(wchar_t *first, wchar_t *last, ssize_t nr_strings = -1)
{
    auto r = std::vector<std::string>{};

    while (first != last) {
        auto it_zero = std::find(first, last, wchar_t{0});
        if (it_zero == last) {
            throw parse_error("Could not find terminating zero of a string.");
        }

        auto ws = std::wstring_view{first, narrow_cast<std::size_t>(it_zero - first)};
        if (ws.empty()) {
            // The list is terminated with an empty string.
            break;
        }

        r.push_back(tt::to_string(ws));

        // Continue after the zero terminator.
        first = it_zero + 1;
    }

    if (nr_strings != -1 && ssize(r) != nr_strings) {
        throw parse_error("Unexpected number of string in list.");
    }

    return r;
}

/** Copy a std::string to new memory.
 * The caller will have to delete [] return value.
 */
[[nodiscard]] inline char *make_cstr(char const *c_str, std::size_t size = -1) noexcept
{
    if (size == -1) {
        size = std::strlen(c_str);
    }

    auto r = new char[size + 1];
    std::memcpy(r, c_str, size + 1);
    return r;
}

/** Copy a std::string to new memory.
 * The caller will have to delete [] return value.
 */
[[nodiscard]] inline char *make_cstr(std::string const &s) noexcept
{
    return make_cstr(s.c_str(), s.size());
}

} // namespace tt::inline v1
