// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "type_traits.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <variant>
#include <type_traits>

namespace hi { inline namespace v1 {

class glob_pattern {
public:
    constexpr glob_pattern() noexcept = default;
    constexpr glob_pattern(glob_pattern const&) noexcept = default;
    constexpr glob_pattern(glob_pattern&&) noexcept = default;
    constexpr glob_pattern& operator=(glob_pattern const&) noexcept = default;
    constexpr glob_pattern& operator=(glob_pattern&&) noexcept = default;

    glob_pattern(std::string_view str) : _tokens(parse(str)) {}

    glob_pattern(std::string const& str) : glob_pattern(std::string_view{str}) {}
    glob_pattern(char const *str) : glob_pattern(std::string_view{str}) {}
    glob_pattern(std::filesystem::path const& path) : glob_pattern(path.generic_string()) {}

    [[nodiscard]] constexpr std::string string() noexcept
    {
        auto r = std::string{};
        for (hilet& token : _tokens) {
            r += token.string();
        }
        return r;
    }

    [[nodiscard]] constexpr std::string debug_string() noexcept
    {
        auto r = std::string{};
        for (hilet& token : _tokens) {
            r += token.debug_string();
        }
        return r;
    }

    [[nodiscard]] constexpr std::string base_string() const noexcept
    {
        if (_tokens.empty() or not _tokens.front().is_text()) {
            return {};
        } else {
            return _tokens.front().string();
        }
    }

    [[nodiscard]] std::filesystem::path base_path() const noexcept
    {
        auto text = base_string();
        if (auto i = text.rfind('/'); i == std::string::npos) {
            // If there is no slash then there is no base directory.
            text.clear();
        } else {
            // Include the trailing slash of the base directory.
            text.resize(i + 1);
        }
        return {std::move(text)};
    }

    [[nodiscard]] constexpr bool matches(std::string_view str) const noexcept
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

    [[nodiscard]] constexpr bool matches(std::string const& str) const noexcept
    {
        return matches(std::string_view{str});
    }

    [[nodiscard]] constexpr bool matches(char const *str) const noexcept
    {
        return matches(std::string_view{str});
    }

    [[nodiscard]] bool matches(std::filesystem::path const& path) const noexcept
    {
        return matches(path.generic_string());
    }

private:
    enum class match_result_type { fail, success, unchecked };

    class token_type {
    public:
        using text_type = std::string;
        using character_class_type = std::vector<std::pair<char, char>>;
        using alternation_type = std::vector<std::string>;
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

        template<bool Left>
        [[nodiscard]] constexpr match_result_type strip(std::string_view& str) const noexcept
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

        [[nodiscard]] constexpr match_result_type matches(std::string_view& str, size_t iteration) const noexcept
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
                    for (auto i = 0_uz; i != std::string_view::npos; i = str.find('/', i + 1)) {
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

        [[nodiscard]] constexpr std::string string() const noexcept
        {
            auto r = std::string{};

            if (auto text_ptr = std::get_if<text_type>(&_value)) {
                r = *text_ptr;

            } else if (auto character_class_ptr = std::get_if<character_class_type>(&_value)) {
                r += '[';
                for (hilet[first_char, last_char] : *character_class_ptr) {
                    if (first_char == last_char) {
                        r += first_char;
                    } else {
                        r += first_char;
                        r += '-';
                        r += last_char;
                    }
                }
                r += ']';

            } else if (auto alternation_ptr = std::get_if<alternation_type>(&_value)) {
                r += '{';
                for (hilet& text : *alternation_ptr) {
                    if (r.size() > 1) {
                        r += ',';
                    }
                    r += text;
                }
                r += '}';

            } else if (std::holds_alternative<any_character_type>(_value)) {
                r += '?';

            } else if (std::holds_alternative<any_text_type>(_value)) {
                r += '*';

            } else if (std::holds_alternative<any_directory_type>(_value)) {
                r += "/**/";

            } else {
                hi_no_default();
            }

            return r;
        }

        [[nodiscard]] constexpr std::string debug_string() const noexcept
        {
            auto r = std::string{};

            if (auto text_ptr = std::get_if<text_type>(&_value)) {
                r += '\'';
                r += *text_ptr;
                r += '\'';

            } else if (auto character_class_ptr = std::get_if<character_class_type>(&_value)) {
                r += '[';
                for (hilet[first_char, last_char] : *character_class_ptr) {
                    if (first_char == last_char) {
                        r += first_char;
                    } else {
                        r += first_char;
                        r += '-';
                        r += last_char;
                    }
                }
                r += ']';

            } else if (auto alternation_ptr = std::get_if<alternation_type>(&_value)) {
                r += '{';
                for (hilet& text : *alternation_ptr) {
                    if (r.size() > 1) {
                        r += ',';
                    }
                    r += text;
                }
                r += '}';

            } else if (std::holds_alternative<any_character_type>(_value)) {
                r += '?';

            } else if (std::holds_alternative<any_text_type>(_value)) {
                r += '*';

            } else if (std::holds_alternative<any_directory_type>(_value)) {
                r += "/**/";

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

        static_assert(std::is_same_v<std::decay_t<decltype(*first)>, char>);

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
                case '/':
                    state = slash;
                    break;
                case '?':
                    HI_GLOB_APPEND_TEXT();
                    r.push_back(make_any_character());
                    break;
                case '*':
                    state = star;
                    break;
                case '[':
                    HI_GLOB_APPEND_TEXT();
                    state = bracket;
                    break;
                case '{':
                    HI_GLOB_APPEND_TEXT();
                    state = brace;
                    break;
                default:
                    text += c;
                }
                break;

            case star:
                if (c == '*') {
                    throw parse_error("Double ** is only allowed between slashes, like /**/.");
                } else {
                    HI_GLOB_APPEND_TEXT();
                    r.push_back(make_any_text());
                    text += c;
                    state = idle;
                }
                break;

            case slash:
                if (c == '*') {
                    state = slash_star;
                } else {
                    text += '/';
                    text += c;
                    state = idle;
                }
                break;

            case slash_star:
                if (c == '*') {
                    state = slash_star_star;
                } else {
                    text += '/';
                    HI_GLOB_APPEND_TEXT();
                    r.push_back(make_any_text());
                    text += c;
                    state = idle;
                }
                break;

            case slash_star_star:
                if (c == '/') {
                    HI_GLOB_APPEND_TEXT();
                    r.push_back(make_any_directory());
                    state = idle;
                } else {
                    throw parse_error("Double ** is only allowed between slashes, like /**/.");
                }
                break;

            case bracket:
                switch (c) {
                case '/':
                    throw parse_error("Slash '/' is not allowed inside a character class, i.e. between '[' and ']'.");
                case '-':
                    if (character_class.empty()) {
                        character_class.emplace_back(c, c);
                    } else {
                        state = bracket_range;
                    }
                    break;
                case ']':
                    r.push_back(make_character_class(std::move(character_class)));
                    character_class.clear();
                    state = idle;
                    break;
                default:
                    character_class.emplace_back(c, c);
                }
                break;

            case bracket_range:
                switch (c) {
                case '/':
                    throw parse_error("Slash '/' is not allowed inside a character class, i.e. between '[' and ']'.");
                case '-':
                    throw parse_error("Double '--' is not allowed inside a character class, i.e. between '[' and ']'.");
                case ']':
                    character_class.emplace_back('-', '-');
                    r.push_back(make_character_class(std::move(character_class)));
                    character_class.clear();
                    state = idle;
                    break;
                default:
                    character_class.back().second = c;
                    state = bracket;
                    break;
                }
                break;

            case brace:
                switch (c) {
                case '/':
                    throw parse_error("Slash '/' is not allowed inside an alternation, i.e. between '{' and '}'.");

                case '}':
                    if (not text.empty()) {
                        alternation.push_back(std::move(text));
                        text.clear();
                    }
                    r.push_back(make_alternation(std::move(alternation)));
                    alternation.clear();
                    state = idle;
                    break;

                case ',':
                    alternation.push_back(std::move(text));
                    text.clear();
                    break;

                default:
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
            text += '/';
            HI_GLOB_APPEND_TEXT();
            break;

        case slash_star:
            text += '/';
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
    [[nodiscard]] constexpr static bool matches_strip(const_iterator& first, const_iterator& last, std::string_view& str) noexcept
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

    [[nodiscard]] constexpr static bool matches_strip(const_iterator& first, const_iterator& last, std::string_view& str) noexcept
    {
        return matches_strip<true>(first, last, str) and matches_strip<false>(first, last, str);
    }

    [[nodiscard]] constexpr bool matches(const_iterator it, const_iterator last, std::string_view original) const noexcept
    {
        hi_axiom(it != last);

        struct stack_element {
            std::string_view str;
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

[[nodiscard]] inline generator<std::filesystem::path> glob(forward_of<glob_pattern> auto&& pattern)
{
    // Make sure we move the pattern when passed an rvalue-reference/temporary.
    forward_copy_or_ref_t<decltype(pattern)> pattern_ = hi_forward(pattern);
    auto path = pattern_.base_path();

    hilet first = std::filesystem::recursive_directory_iterator(path);
    hilet last = std::filesystem::recursive_directory_iterator();
    for (auto it = first; it != last; ++it) {
        if (pattern_.matches(it->path())) {
            co_yield it->path();
        }
    }
}

[[nodiscard]] inline generator<std::filesystem::path> glob(std::string_view pattern)
{
    return glob(glob_pattern{pattern});
}

[[nodiscard]] inline generator<std::filesystem::path> glob(std::string const& pattern)
{
    return glob(glob_pattern{pattern});
}

[[nodiscard]] inline generator<std::filesystem::path> glob(char const *pattern)
{
    return glob(glob_pattern{pattern});
}

[[nodiscard]] inline generator<std::filesystem::path> glob(std::filesystem::path const& pattern)
{
    return glob(glob_pattern{pattern});
}

[[nodiscard]] inline generator<std::filesystem::path> glob(URL const& pattern)
{
    return glob(glob_pattern{pattern.filesystem_path()});
}
}} // namespace hi::v1
