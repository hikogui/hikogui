// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../char_maps/char_maps.hpp"
#include "algorithm_misc.hpp"
#include "../macros.hpp"
#include <string>
#include <string_view>
#include <iterator>
#include <vector>
#include <tuple>
#include <type_traits>
#include <cstdlib>
#include <bit>

hi_export_module(hikogui.algorithm.strings);

hi_warning_push();
// C26409: Avoid calling new and delete explicitly, use std::make_unique<T> instead (r.11).
// make_cstr() is used for arguments passed to tt_main() for compatibility we need to create
// those using new/delete.
hi_warning_ignore_msvc(26409);

hi_export namespace hi::inline v1 {

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

[[nodiscard]] constexpr bool is_digit(std::string_view str) noexcept
{
    for (hilet c : str) {
        if (not is_digit(c)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] constexpr bool is_alpha(std::string_view str) noexcept
{
    for (hilet c : str) {
        if (not is_alpha(c)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] constexpr char to_lower(char c) noexcept
{
    return (c >= 'A' and c <= 'Z') ? (c - 'A') + 'a' : c;
}

[[nodiscard]] constexpr char to_upper(char c) noexcept
{
    return (c >= 'a' and c <= 'z') ? (c - 'a') + 'A' : c;
}

[[nodiscard]] hi_inline std::string to_lower(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (hilet c : str) {
        r += to_lower(c);
    }

    return r;
}

[[nodiscard]] hi_inline std::string to_upper(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (hilet c : str) {
        r += to_upper(c);
    }

    return r;
}

/** Convert the current string to using title case.
 *
 * This function does not do full unicode case conversion;
 * only ASCII letters [a-zA-Z] will be modified.
 */
[[nodiscard]] constexpr std::string to_title(std::string_view rhs) noexcept
{
    auto r = std::string{rhs};

    bool first = true;
    for (auto& c : r) {
        if (first) {
            c = to_upper(c);
            first = false;
        } else if (c == ' ') {
            first = true;
        } else {
            c = to_lower(c);
        }
    }

    return r;
}

/** Convert the current string to using title case.
 *
 * This function does not do full unicode case conversion;
 * only ASCII letters [a-zA-Z] will be modified.
 */
template<size_t N>
[[nodiscard]] constexpr fixed_string<N> to_title(fixed_string<N> const &rhs) noexcept
{
    auto r = rhs;

    bool first = true;
    for (auto &c: r) {
        if (first) {
            c = to_upper(c);
            first = false;
        } else if (c == ' ') {
            first = true;
        } else {
            c = to_lower(c);
        }
    }

    return r;
}

/** Normalize string to use only line-feeds.
 */
[[nodiscard]] hi_inline std::string normalize_lf(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    auto found_cr = false;
    for (hilet c : str) {
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
[[nodiscard]] hi_inline std::string make_identifier(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    r += is_name_first(str.front()) ? str.front() : '_';
    for (hilet c : str.substr(1)) {
        r += is_name_next(c) ? c : '_';
    }

    return r;
}

/** Create a slug from a string.
 * A slug contains only lower case letters, digits and dashes.
 * Duplicated dashes are eliminated.
 */
[[nodiscard]] constexpr std::string make_slug(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    std::size_t dash_count = 0;
    for (hilet c : str) {
        if (is_alpha_num(c)) {
            dash_count = 0;
            r += to_lower(c);
        } else if (dash_count++ == 0) {
            r += '-';
        }
    }

    return r;
}

[[nodiscard]] constexpr bool is_slug(std::string_view str) noexcept
{
    for (hilet c : str) {
        if (not (is_alpha_num(c) or c == '-')) {
            return false;
        }
    }
    return true;
}

/** Create a title from a string.
 * A title contains words separated by a single space, where each word starts with
 * a capital letter followed by lower case letters. Digits may be part of a word
 * or form a separate word, digits are not counted as the start of a word for
 * capitalization.
 */
[[nodiscard]] hi_inline std::string make_title(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    // Do not start with a space.
    std::size_t space_count = 1;
    std::size_t letter_count = 0;
    for (hilet c : str) {
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

template<typename T, size_t N>
[[nodiscard]] constexpr uint32_t fourcc(T const (&txt)[N]) noexcept requires(sizeof(T) == 1 and (N == 4 or N == 5))
{
    auto r = uint32_t{};
    r |= truncate<uint8_t>(txt[0]);
    r <<= 8;
    r |= truncate<uint8_t>(txt[1]);
    r <<= 8;
    r |= truncate<uint8_t>(txt[2]);
    r <<= 8;
    r |= truncate<uint8_t>(txt[3]);

    if constexpr (N == 5) {
        hi_axiom(txt[4] == 0);
    }
    return r;
}

[[nodiscard]] constexpr uint32_t fourcc_from_cstr(char const *txt) noexcept
{
    hi_assert_not_null(txt);
    return
        (char_cast<uint32_t>(txt[0]) << 24) |
        (char_cast<uint32_t>(txt[1]) << 16) |
        (char_cast<uint32_t>(txt[2]) << 8) |
        char_cast<uint32_t>(txt[3]);
}

[[nodiscard]] hi_inline std::string fourcc_to_string(uint32_t x) noexcept
{
    auto r = std::string{};
    r += truncate<char>((x >> 24) & 0xff);
    r += truncate<char>((x >> 16) & 0xff);
    r += truncate<char>((x >> 8) & 0xff);
    r += truncate<char>(x & 0xff);
    return r;
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
string_find_any(std::string_view haystack, std::size_t pos, FirstNeedle const& first_needle, Needles const&...needles) noexcept
{
    using std::size;

    std::size_t first = haystack.find(first_needle, pos);
    std::size_t last = first + string_size(first_needle);

    if (first == std::string_view::npos) {
        first = size(haystack);
        last = size(haystack);
    }

    if constexpr (sizeof...(Needles) != 0) {
        hilet[other_first, other_last] = string_find_any(haystack, pos, needles...);
        if (other_first < first) {
            first = other_first;
            last = other_last;
        }
    }

    return {first, last};
}

template<typename StringType, typename... Needles>
[[nodiscard]] std::vector<StringType> _split(std::string_view haystack, Needles const&...needles) noexcept
{
    auto r = std::vector<StringType>{};

    std::string_view::size_type current_pos = 0;

    while (current_pos < size(haystack)) {
        hilet[needle_first, needle_last] = string_find_any(haystack, current_pos, needles...);
        r.push_back(StringType{haystack.substr(current_pos, needle_first - current_pos)});
        current_pos = needle_last;
    }

    return r;
}

template<typename... Needles>
[[nodiscard]] std::vector<std::string> split(std::string_view haystack, Needles const&...needles) noexcept
{
    return _split<std::string>(haystack, needles...);
}

[[nodiscard]] hi_inline std::vector<std::string> split(std::string_view haystack) noexcept
{
    return split(haystack, ' ');
}

template<typename... Needles>
[[nodiscard]] std::vector<std::string_view> split_view(std::string_view haystack, Needles const&...needles) noexcept
{
    return _split<std::string_view>(haystack, needles...);
}

[[nodiscard]] hi_inline std::vector<std::string_view> split_view(std::string_view haystack) noexcept
{
    return split_view(haystack, ' ');
}

template<typename CharT>
[[nodiscard]] std::basic_string<CharT>
join(std::vector<std::basic_string<CharT>> const& list, std::basic_string_view<CharT> const joiner = {}) noexcept
{
    std::string r;

    if (list.size() > 1) {
        std::size_t final_size = (list.size() - 1) * joiner.size();
        for (hilet& item : list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    std::size_t i = 0;
    for (hilet& item : list) {
        if (i++ != 0) {
            r += joiner;
        }
        r += item;
    }
    return r;
}

template<typename CharT>
[[nodiscard]] std::basic_string<CharT>
join(std::vector<std::basic_string<CharT>> const& list, std::basic_string<CharT> const& joiner) noexcept
{
    return join(list, std::basic_string_view<CharT>{joiner});
}

template<typename CharT>
[[nodiscard]] std::basic_string<CharT> join(std::vector<std::basic_string<CharT>> const& list, CharT const *joiner) noexcept
{
    return join(list, std::basic_string_view<CharT>{joiner});
}

[[nodiscard]] hi_inline std::string join(std::vector<std::string_view> const& list, std::string_view const joiner = {}) noexcept
{
    std::string r;

    if (list.size() > 1) {
        std::size_t final_size = (list.size() - 1) * joiner.size();
        for (hilet item : list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    int64_t i = 0;
    for (hilet item : list) {
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
[[nodiscard]] hi_inline std::pair<int, int> count_line_and_columns(It begin, It const end)
{
    int line = 1;
    int column = 1;

    for (; begin != end; begin++) {
        switch (*begin) {
        case '\n':
            line++;
            [[fallthrough]];
        case '\r':
            column = 1;
            break;
        case '\t':
            column = ((((column - 1) / 8) + 1) * 8) + 1;
            break;
        default:
            column++;
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

[[nodiscard]] hi_inline std::string lstrip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto first = front_strip(begin(haystack), end(haystack), begin(needle), end(needle));
    return std::string{first, end(haystack)};
}

[[nodiscard]] hi_inline std::string rstrip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto last = back_strip(begin(haystack), end(haystack), begin(needle), end(needle));
    return std::string{begin(haystack), last};
}

[[nodiscard]] hi_inline std::string strip(std::string_view haystack, std::string needle = " \t\r\n\f") noexcept
{
    auto first = front_strip(begin(haystack), end(haystack), begin(needle), end(needle));
    auto last = back_strip(first, end(haystack), begin(needle), end(needle));
    return std::string{first, last};
}

} // namespace hi::inline v1

hi_warning_pop();
