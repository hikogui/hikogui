// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

/** @file file/glob.hpp Defines utilities for handling glob patterns.
 * @ingroup file
 */

#include "path_location.hpp"
#include "../char_maps/to_string.hpp"
#include "../utility.hpp"
#include "../type_traits.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <variant>
#include <type_traits>

namespace hi { inline namespace v1 {

/** A glob pattern.
 * @ingroup file
 *
 * A glob algorithm is used for matching with filenames and directories.
 * Glob may also be used on strings that do not involve the filesystem at all,
 * however certain tokens implicitly include or exclude the slash '/' character.
 *
 *  Token          | Description
 * --------------- | --------------------------------
 *  foo            | Matches the text "foo".
 *  ?              | Matches any single code-unit except '/'.
 *  [abcd]         | Matches a single code-unit that is 'a', 'b', 'c' or 'd'.
 *  [a-d]          | Matches a single code-unit that is 'a', 'b', 'c' or 'd'.
 *  [-a-d]         | Matches a single code-unit that is '-', 'a', 'b', 'c' or 'd'.
 *  {foo,bar,baz}  | Matches the text "foo", "bar" or "baz".
 *  *              | Matches zero or more code-units except '/'.
 *  / ** /         | Matches A single slash '/' or zero or more code-units between two slashes '/'.
 */
class glob_pattern {
public:
    constexpr glob_pattern() noexcept = default;
    constexpr glob_pattern(glob_pattern const&) noexcept = default;
    constexpr glob_pattern(glob_pattern&&) noexcept = default;
    constexpr glob_pattern& operator=(glob_pattern const&) noexcept = default;
    constexpr glob_pattern& operator=(glob_pattern&&) noexcept = default;

    /** Parse a string to a glob-pattern.
     *
     * @param str The string to be parsed.
     */
    glob_pattern(std::u32string_view str) : _tokens(parse(str)) {}

    /** Parse a string to a glob-pattern.
     *
     * @param str The string to be parsed.
     */
    glob_pattern(std::u32string const& str) : glob_pattern(std::u32string_view{str}) {}

    /** Parse a string to a glob-pattern.
     *
     * @param str The string to be parsed.
     */
    glob_pattern(char32_t const *str) : glob_pattern(std::u32string_view{str}) {}

    /** Parse a string to a glob-pattern.
     *
     * @param str The string to be parsed.
     */
    glob_pattern(std::string_view str) : glob_pattern(hi::to_u32string(str)) {}

    /** Parse a string to a glob-pattern.
     *
     * @param str The string to be parsed.
     */
    glob_pattern(std::string const& str) : glob_pattern(std::string_view{str}) {}

    /** Parse a string to a glob-pattern.
     *
     * @param str The string to be parsed.
     */

    glob_pattern(char const *str) : glob_pattern(std::string_view{str}) {}

    /** Parse a path to a glob-pattern.
     *
     * @param path The path to be parsed.
     */
    glob_pattern(std::filesystem::path const& path) : glob_pattern(path.generic_u32string()) {}

    /** Convert a glob-pattern to a string.
     *
     * @return The string representing the pattern
     */
    [[nodiscard]] constexpr std::u32string u32string() noexcept
    {
        auto r = std::u32string{};
        for (hilet& token : _tokens) {
            r += token.u32string();
        }
        return r;
    }

    /** Convert a glob-pattern to a string.
     *
     * @return The string representing the pattern
     */
    [[nodiscard]] constexpr std::string string() noexcept
    {
        return hi::to_string(u32string());
    }

    /** Convert a glob-pattern to a debug-string.
     *
     * This function is used for debugging the glob parser, and for using in unit-tests.
     *
     * @return The string representing the pattern
     */
    [[nodiscard]] constexpr std::u32string debug_u32string() noexcept
    {
        auto r = std::u32string{};
        for (hilet& token : _tokens) {
            r += token.debug_u32string();
        }
        return r;
    }

    /** Convert a glob-pattern to a debug-string.
     *
     * This function is used for debugging the glob parser, and for using in unit-tests.
     *
     * @return The string representing the pattern
     */
    [[nodiscard]] constexpr std::string debug_string() noexcept
    {
        return hi::to_string(debug_u32string());
    }

    /** Get the initial fixed part of the pattern.
     *
     * This gets the initial part of the pattern that is fixed,
     * this is used for as a starting point for a search.
     *
     * For example by getting the base_string you can use a binary-search
     * into a sorted list of strings, then once you find a string you can
     * iterate over the list and glob-match each string.
     *
     * @return The initial fixed part of the pattern.
     */
    [[nodiscard]] constexpr std::u32string base_u32string() const noexcept
    {
        if (_tokens.empty() or not _tokens.front().is_text()) {
            return {};
        } else {
            auto r = _tokens.front().u32string();
            if (_tokens.size() >= 2 and _tokens[1].is_any_directory()) {
                // An any_directory_type always includes at least 1 slash.
                r += U'/';
            }
            return r;
        }
    }

    /** Get the initial fixed part of the pattern.
     *
     * This gets the initial part of the pattern that is fixed,
     * this is used for as a starting point for a search.
     *
     * For example by getting the base_string you can use a binary-search
     * into a sorted list of strings, then once you find a string you can
     * iterate over the list and glob-match each string.
     *
     * @return The initial fixed part of the pattern.
     */
    [[nodiscard]] constexpr std::string base_string() const noexcept
    {
        return to_string(base_u32string());
    }

    /** Get the initial path of the pattern.
     *
     * This gets the initial path of the pattern that is fixed,
     * this is used for as a starting point for a search.
     *
     * For example this will be the directory where to start
     * recursively iterating on.
     *
     * @return The initial fixed path of the pattern.
     */
    [[nodiscard]] std::filesystem::path base_path() const noexcept
    {
        auto text = base_u32string();
        if (auto i = text.rfind('/'); i == std::u32string::npos) {
            // If there is no slash then there is no base directory.
            text.clear();
        } else {
            // Include the trailing slash of the base directory.
            text.resize(i + 1);
        }
        return {std::move(text)};
    }

    /** Match the pattern with the given string.
     *
     * @param str The string to match with this pattern.
     * @return True if the string matches the pattern.
     */
    [[nodiscard]] constexpr bool matches(std::u32string_view str) const noexcept
    {
        auto first = _tokens.cbegin();
        auto last = _tokens.cend();

        // Strip away the prefix and suffix quickly.
        if (not matches_strip(first, last, str)) {
            // The prefix and suffix of the do not match.
            return false;

        } else if (first == last) {
            // All tokens matched the prefix and suffix.
            // If the resulting string is empty than it is a match.
            return str.empty();
        }

        // Do more complex matching with the stripped string.
        return matches(first, last, str);
    }

    /** Match the pattern with the given string.
     *
     * @param str The string to match with this pattern.
     * @return True if the string matches the pattern.
     */
    [[nodiscard]] constexpr bool matches(std::u32string const& str) const noexcept
    {
        return matches(std::u32string_view{str});
    }

    /** Match the pattern with the given string.
     *
     * @param str The string to match with this pattern.
     * @return True if the string matches the pattern.
     */
    [[nodiscard]] constexpr bool matches(char32_t const *str) const noexcept
    {
        return matches(std::u32string_view{str});
    }

    /** Match the pattern with the given string.
     *
     * @param str The string to match with this pattern.
     * @return True if the string matches the pattern.
     */
    [[nodiscard]] constexpr bool matches(std::string_view str) const noexcept
    {
        return matches(to_u32string(str));
    }

    /** Match the pattern with the given string.
     *
     * @param str The string to match with this pattern.
     * @return True if the string matches the pattern.
     */
    [[nodiscard]] constexpr bool matches(std::string const& str) const noexcept
    {
        return matches(std::string_view{str});
    }

    /** Match the pattern with the given string.
     *
     * @param str The string to match with this pattern.
     * @return True if the string matches the pattern.
     */
    [[nodiscard]] constexpr bool matches(char const *str) const noexcept
    {
        return matches(std::string_view{str});
    }

    /** Match the pattern with the given path.
     *
     * @param path The path to match with this pattern.
     * @return True if the path matches the pattern.
     */
    [[nodiscard]] bool matches(std::filesystem::path const& path) const noexcept
    {
        return matches(path.generic_u32string());
    }

private:
    enum class match_result_type { fail, success, unchecked };

    class token_type {
    public:
        using text_type = std::u32string;
        using character_class_type = std::vector<std::pair<char32_t, char32_t>>;
        using alternation_type = std::vector<std::u32string>;
        class any_character_type {};
        class any_text_type {};
        class any_directory_type {};

        constexpr token_type(token_type const&) noexcept = default;
        constexpr token_type(token_type&&) noexcept = default;
        constexpr token_type& operator=(token_type const&) noexcept = default;
        constexpr token_type& operator=(token_type&&) noexcept = default;

        constexpr token_type(text_type&& rhs) noexcept : _value(std::move(rhs)) {}
        constexpr token_type(character_class_type&& rhs) noexcept : _value(std::move(rhs)) {}
        constexpr token_type(alternation_type&& rhs) noexcept : _value(std::move(rhs)) {}
        constexpr token_type(any_character_type&& rhs) noexcept : _value(std::move(rhs)) {}
        constexpr token_type(any_text_type&& rhs) noexcept : _value(std::move(rhs)) {}
        constexpr token_type(any_directory_type&& rhs) noexcept : _value(std::move(rhs)) {}

        [[nodiscard]] constexpr bool is_text() const noexcept
        {
            return std::holds_alternative<text_type>(_value);
        }

        [[nodiscard]] constexpr bool is_any_directory() const noexcept
        {
            return std::holds_alternative<any_directory_type>(_value);
        }

        template<bool Left>
        [[nodiscard]] constexpr match_result_type strip(std::u32string_view& str) const noexcept
        {
            constexpr bool Right = not Left;

            if (hilet text_ptr = std::get_if<text_type>(&_value)) {
                if (Left and str.starts_with(*text_ptr)) {
                    str.remove_prefix(text_ptr->size());
                    return match_result_type::success;

                } else if (Right and str.ends_with(*text_ptr)) {
                    str.remove_suffix(text_ptr->size());
                    return match_result_type::success;

                } else {
                    return match_result_type::fail;
                }

            } else if (hilet character_class_ptr = std::get_if<character_class_type>(&_value)) {
                if (str.empty()) {
                    return match_result_type::fail;
                }
                hilet c = Left ? str.front() : str.back();
                for (hilet[first_char, last_char] : *character_class_ptr) {
                    if (c >= first_char and c <= last_char) {
                        if constexpr (Left) {
                            str.remove_prefix(1);
                        } else {
                            str.remove_suffix(1);
                        }
                        return match_result_type::success;
                    }
                }
                return match_result_type::fail;

            } else if (std::holds_alternative<any_character_type>(_value)) {
                if (str.empty()) {
                    return match_result_type::fail;
                }
                if constexpr (Left) {
                    str.remove_prefix(1);
                } else {
                    str.remove_suffix(1);
                }
                return match_result_type::success;

            } else {
                return match_result_type::unchecked;
            }
        }

        [[nodiscard]] constexpr match_result_type matches(std::u32string_view& str, size_t iteration) const noexcept
        {
            if (hilet text_ptr = std::get_if<text_type>(&_value)) {
                if (iteration != 0) {
                    return match_result_type::fail;

                } else if (str.starts_with(*text_ptr)) {
                    str.remove_prefix(text_ptr->size());
                    return match_result_type::success;

                } else {
                    return match_result_type::fail;
                }

            } else if (hilet character_class_ptr = std::get_if<character_class_type>(&_value)) {
                if (iteration != 0 or str.empty()) {
                    return match_result_type::fail;
                } else {
                    hilet c = str.front();
                    for (hilet[first_char, last_char] : *character_class_ptr) {
                        if (c >= first_char and c <= last_char) {
                            str.remove_prefix(1);
                            return match_result_type::success;
                        }
                    }
                    return match_result_type::fail;
                }

            } else if (hilet alternation_ptr = std::get_if<alternation_type>(&_value)) {
                if (iteration >= alternation_ptr->size()) {
                    return match_result_type::fail;
                } else if (str.starts_with((*alternation_ptr)[iteration])) {
                    str.remove_prefix((*alternation_ptr)[iteration].size());
                    return match_result_type::success;
                } else {
                    return match_result_type::unchecked;
                }

            } else if (std::holds_alternative<any_character_type>(_value)) {
                if (iteration != 0 or str.empty()) {
                    return match_result_type::fail;
                } else {
                    str.remove_prefix(1);
                    return match_result_type::success;
                }

            } else if (std::holds_alternative<any_text_type>(_value)) {
                if (iteration > str.size() or iteration > str.find('/')) {
                    return match_result_type::fail;
                } else {
                    str.remove_prefix(iteration);
                    return match_result_type::success;
                }

            } else if (std::holds_alternative<any_directory_type>(_value)) {
                if (str.empty() or str.front() != '/') {
                    return match_result_type::fail;
                } else {
                    for (auto i = 0_uz; i != std::u32string_view::npos; i = str.find('/', i + 1)) {
                        if (iteration-- == 0) {
                            str.remove_prefix(i + 1);
                            return match_result_type::success;
                        }
                    }
                    return match_result_type::fail;
                }

            } else {
                hi_no_default();
            }
        }

        [[nodiscard]] constexpr std::u32string u32string() const noexcept
        {
            auto r = std::u32string{};

            if (auto text_ptr = std::get_if<text_type>(&_value)) {
                r = *text_ptr;

            } else if (auto character_class_ptr = std::get_if<character_class_type>(&_value)) {
                r += U'[';
                for (hilet[first_char, last_char] : *character_class_ptr) {
                    if (first_char == last_char) {
                        r += first_char;
                    } else {
                        r += first_char;
                        r += U'-';
                        r += last_char;
                    }
                }
                r += U']';

            } else if (auto alternation_ptr = std::get_if<alternation_type>(&_value)) {
                r += U'{';
                for (hilet& text : *alternation_ptr) {
                    if (r.size() > 1) {
                        r += U',';
                    }
                    r += text;
                }
                r += U'}';

            } else if (std::holds_alternative<any_character_type>(_value)) {
                r += U'?';

            } else if (std::holds_alternative<any_text_type>(_value)) {
                r += U'*';

            } else if (std::holds_alternative<any_directory_type>(_value)) {
                r += U"/**/";

            } else {
                hi_no_default();
            }

            return r;
        }

        [[nodiscard]] constexpr std::u32string debug_u32string() const noexcept
        {
            auto r = std::u32string{};

            if (auto text_ptr = std::get_if<text_type>(&_value)) {
                r += U'\'';
                r += *text_ptr;
                r += U'\'';

            } else if (auto character_class_ptr = std::get_if<character_class_type>(&_value)) {
                r += U'[';
                for (hilet[first_char, last_char] : *character_class_ptr) {
                    if (first_char == last_char) {
                        r += first_char;
                    } else {
                        r += first_char;
                        r += U'-';
                        r += last_char;
                    }
                }
                r += U']';

            } else if (auto alternation_ptr = std::get_if<alternation_type>(&_value)) {
                r += U'{';
                for (hilet& text : *alternation_ptr) {
                    if (r.size() > 1) {
                        r += U',';
                    }
                    r += text;
                }
                r += U'}';

            } else if (std::holds_alternative<any_character_type>(_value)) {
                r += U'?';

            } else if (std::holds_alternative<any_text_type>(_value)) {
                r += U'*';

            } else if (std::holds_alternative<any_directory_type>(_value)) {
                r += U"/**/";

            } else {
                hi_no_default();
            }

            return r;
        }

    private:
        using variant_type = std::
            variant<text_type, character_class_type, alternation_type, any_character_type, any_text_type, any_directory_type>;

        variant_type _value;
    };

    using tokens_type = std::vector<token_type>;
    using const_iterator = tokens_type::const_iterator;

    tokens_type _tokens;

    [[nodiscard]] constexpr static token_type make_text(token_type::text_type&& rhs) noexcept
    {
        return token_type{std::move(rhs)};
    }

    [[nodiscard]] constexpr static token_type make_alternation(token_type::alternation_type&& rhs) noexcept
    {
        return token_type{std::move(rhs)};
    }

    [[nodiscard]] constexpr static token_type make_character_class(token_type::character_class_type&& rhs) noexcept
    {
        return token_type{std::move(rhs)};
    }

    [[nodiscard]] constexpr static token_type make_any_character() noexcept
    {
        return token_type{token_type::any_character_type{}};
    }

    [[nodiscard]] constexpr static token_type make_any_text() noexcept
    {
        return token_type{token_type::any_text_type{}};
    }

    [[nodiscard]] constexpr static token_type make_any_directory() noexcept
    {
        return token_type{token_type::any_directory_type{}};
    }

    [[nodiscard]] constexpr static tokens_type parse(auto first, auto last)
    {
#define HI_GLOB_APPEND_TEXT() \
    do { \
        if (not text.empty()) { \
            r.emplace_back(make_text(std::move(text))); \
            text.clear(); \
        } \
    } while (false)

        enum class state_type { idle, star, slash, slash_star, slash_star_star, bracket, bracket_range, brace };
        using enum state_type;

        static_assert(std::is_same_v<std::decay_t<decltype(*first)>, char32_t>);

        auto r = tokens_type{};

        auto state = idle;
        auto text = token_type::text_type{};
        auto alternation = token_type::alternation_type{};
        auto character_class = token_type::character_class_type{};

        auto it = first;
        while (it != last) {
            auto c = *it;
            switch (state) {
            case idle:
                switch (c) {
                case U'/':
                    state = slash;
                    break;
                case U'?':
                    HI_GLOB_APPEND_TEXT();
                    r.push_back(make_any_character());
                    break;
                case U'*':
                    state = star;
                    break;
                case U'[':
                    HI_GLOB_APPEND_TEXT();
                    state = bracket;
                    break;
                case U'{':
                    HI_GLOB_APPEND_TEXT();
                    state = brace;
                    break;
                default:
                    text += c;
                }
                break;

            case star:
                if (c == U'*') {
                    throw parse_error("Double ** is only allowed between slashes, like /**/.");
                } else {
                    HI_GLOB_APPEND_TEXT();
                    r.push_back(make_any_text());
                    text += c;
                    state = idle;
                }
                break;

            case slash:
                if (c == U'*') {
                    state = slash_star;
                } else {
                    text += U'/';
                    text += c;
                    state = idle;
                }
                break;

            case slash_star:
                if (c == U'*') {
                    state = slash_star_star;
                } else {
                    text += U'/';
                    HI_GLOB_APPEND_TEXT();
                    r.push_back(make_any_text());
                    text += c;
                    state = idle;
                }
                break;

            case slash_star_star:
                if (c == U'/') {
                    HI_GLOB_APPEND_TEXT();
                    r.push_back(make_any_directory());
                    state = idle;
                } else {
                    throw parse_error("Double ** is only allowed between slashes, like /**/.");
                }
                break;

            case bracket:
                if (c == U'-') {
                    if (character_class.empty()) {
                        character_class.emplace_back(c, c);
                    } else {
                        state = bracket_range;
                    }
                } else if (c == U']') {
                    r.push_back(make_character_class(std::move(character_class)));
                    character_class.clear();
                    state = idle;
                } else {
                    character_class.emplace_back(c, c);
                }
                break;

            case bracket_range:
                if (c == U'-') {
                    throw parse_error("Double '--' is not allowed inside a character class, i.e. between '[' and ']'.");
                } else if (c == U']') {
                    character_class.emplace_back(U'-', U'-');
                    r.push_back(make_character_class(std::move(character_class)));
                    character_class.clear();
                    state = idle;
                } else {
                    character_class.back().second = c;
                    state = bracket;
                }
                break;

            case brace:
                if (c == U'}') {
                    if (not text.empty()) {
                        alternation.push_back(std::move(text));
                        text.clear();
                    }
                    r.push_back(make_alternation(std::move(alternation)));
                    alternation.clear();
                    state = idle;
                } else if (c == U',') {
                    alternation.push_back(std::move(text));
                    text.clear();
                } else {
                    text += c;
                }
                break;

            default:
                hi_no_default();
            }

            ++it;
        }

        switch (state) {
        case idle:
            HI_GLOB_APPEND_TEXT();
            break;

        case star:
            HI_GLOB_APPEND_TEXT();
            r.push_back(make_any_text());
            break;

        case slash:
            text += U'/';
            HI_GLOB_APPEND_TEXT();
            break;

        case slash_star:
            text += U'/';
            HI_GLOB_APPEND_TEXT();
            r.push_back(make_any_text());
            break;

        case slash_star_star:
            HI_GLOB_APPEND_TEXT();
            r.push_back(make_any_directory());
            break;

        case bracket:
            throw parse_error("Unclosed bracket '[' found in glob pattern.");

        case bracket_range:
            throw parse_error("Unclosed bracket '[' found in glob pattern.");

        case brace:
            throw parse_error("Unclosed brace '{' found in glob pattern.");
        }

        return r;

#undef HI_GLOB_APPEND_TEXT
    }

    [[nodiscard]] constexpr static tokens_type parse(auto&& range)
    {
        return parse(std::ranges::begin(range), std::ranges::end(range));
    }

    template<bool Left>
    [[nodiscard]] constexpr static bool
    matches_strip(const_iterator& first, const_iterator& last, std::u32string_view& str) noexcept
    {
        while (first != last) {
            hilet it = Left ? first : last - 1;
            switch (it->strip<Left>(str)) {
            case match_result_type::fail:
                return false;
            case match_result_type::unchecked:
                return true;
            case match_result_type::success:
                if constexpr (Left) {
                    ++first;
                } else {
                    --last;
                }
                break;
            default:
                hi_no_default();
            }
        }
        return str.empty();
    }

    [[nodiscard]] constexpr static bool
    matches_strip(const_iterator& first, const_iterator& last, std::u32string_view& str) noexcept
    {
        return matches_strip<true>(first, last, str) and matches_strip<false>(first, last, str);
    }

    [[nodiscard]] constexpr bool matches(const_iterator it, const_iterator last, std::u32string_view original) const noexcept
    {
        hi_assert(it != last);

        struct stack_element {
            std::u32string_view str;
            size_t iteration;
        };

        auto stack = std::vector<stack_element>{};
        stack.reserve(std::distance(it, last));

        stack.emplace_back(original, 0);
        while (true) {
            auto [str, iteration] = stack.back();

            switch (it->matches(str, iteration)) {
            case match_result_type::success:
                if (it + 1 == last) {
                    if (str.empty()) {
                        // This token fully matches the whole string.
                        return true;

                    } else {
                        // This token matches, but this is the last token.
                        // Try the next iteration on this token.
                        ++stack.back().iteration;
                    }

                } else {
                    // This token matches, test the next token.
                    stack.emplace_back(str, 0);
                    ++it;
                }
                break;

            case match_result_type::unchecked:
                // This iteration of the token did not match, try the next iteration.
                ++stack.back().iteration;
                break;

            case match_result_type::fail:
                // None of the token iterations succeeded.
                if (stack.size() == 1) {
                    // Can't track further back; the complete match failed.
                    return false;

                } else {
                    // Track back and try the next iteration of the previous token.
                    stack.pop_back();
                    ++stack.back().iteration;
                    --it;
                }
                break;

            default:
                hi_no_default();
            }
        }
    }
};

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param pattern The pattern to search the filesystem for.
 * @return a generator yielding paths to objects on the filesystem that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(glob_pattern pattern)
{
    auto path = pattern.base_path();

    hilet first = std::filesystem::recursive_directory_iterator(path);
    hilet last = std::filesystem::recursive_directory_iterator();
    for (auto it = first; it != last; ++it) {
        hilet& iterated_path = it->path();
        if (pattern.matches(iterated_path)) {
            co_yield iterated_path;
        }
    }
}

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param pattern The pattern to search the filesystem for.
 * @return a generator yielding paths to objects on the filesystem that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(std::string_view pattern)
{
    return glob(glob_pattern{std::move(pattern)});
}

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param pattern The pattern to search the filesystem for.
 * @return a generator yielding paths to objects on the filesystem that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(std::string pattern)
{
    return glob(glob_pattern{std::move(pattern)});
}

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param pattern The pattern to search the filesystem for.
 * @return a generator yielding paths to objects on the filesystem that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(char const *pattern)
{
    return glob(glob_pattern{pattern});
}

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param pattern The pattern to search the filesystem for.
 * @return a generator yielding paths to objects on the filesystem that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(std::filesystem::path pattern)
{
    return glob(glob_pattern{std::move(pattern)});
}

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param location The path-location to search files in
 * @param ref A relative path pattern to search the path-location
 * @return a generator yielding paths to objects in the path-location that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(path_location location, std::filesystem::path ref)
{
    for (hilet& directory : get_paths(location)) {
        for (hilet& path : glob(directory / ref)) {
            co_yield path;
        }
    }
}

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param location The path-location to search files in
 * @param ref A relative path pattern to search the path-location
 * @return a generator yielding paths to objects in the path-location that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(path_location location, std::string_view ref)
{
    return glob(location, std::filesystem::path{std::move(ref)});
}

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param location The path-location to search files in
 * @param ref A relative path pattern to search the path-location
 * @return a generator yielding paths to objects in the path-location that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(path_location location, std::string ref)
{
    return glob(location, std::filesystem::path{std::move(ref)});
}

/** Find paths on the filesystem that match the glob pattern.
 * @ingroup file
 *
 * @param location The path-location to search files in
 * @param ref A relative path pattern to search the path-location
 * @return a generator yielding paths to objects in the path-location that match the pattern.
 */
[[nodiscard]] inline generator<std::filesystem::path> glob(path_location location, char const *ref)
{
    return glob(location, std::filesystem::path{ref});
}

}} // namespace hi::v1
