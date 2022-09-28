// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <variant>
#include <type_traits>
#include <regex>

namespace hi { inline namespace v1 {

class glob_pattern {
public:
    constexpr glob_pattern() noexcept = default;
    constexpr glob_pattern(glob_pattern const&) noexcept = default;
    constexpr glob_pattern(glob_pattern&&) noexcept = default;
    constexpr glob_pattern& operator=(glob_pattern const&) noexcept = default;
    constexpr glob_pattern& operator=(glob_pattern&&) noexcept = default;

    glob_pattern(std::string_view str) : _tokens(parse(str)), _regex()
    {
        _regex = make_regex(_tokens);
    }

    glob_pattern(std::string const& str) : glob_pattern(std::string_view{str}) {}
    glob_pattern(char const *str) : glob_pattern(std::string_view{str}) {}
    glob_pattern(std::filesystem::path const& path) : glob_pattern(path.generic_string()) {}

    [[nodiscard]] constexpr friend std::string to_string(glob_pattern const& rhs) noexcept
    {
        auto r = std::string{};

        for (hilet& token : rhs._tokens) {
            if (auto string_ = std::get_if<string_type>(&token)) {
                r += *string_;
            } else if (std::holds_alternative<separator_type>(token)) {
                r += '/';
            } else if (auto strings_ = std::get_if<strings_type>(&token)) {
                r += '{';
                for (hilet& s : *strings_) {
                    r += s;
                    r += ',';
                }
                r += '}';
            } else if (std::holds_alternative<any_char_type>(token)) {
                r += '?';
            } else if (std::holds_alternative<any_string_type>(token)) {
                r += '*';
            } else if (std::holds_alternative<any_directory_type>(token)) {
                r += "/**";
            } else {
                hi_no_default();
            }
        }
        return r;
    }

    [[nodiscard]] constexpr friend std::string to_regex_string(glob_pattern const& rhs) noexcept
    {
        return make_regex_string(rhs._tokens);
    }

    [[nodiscard]] constexpr std::string base_generic_string() const noexcept
    {
        auto r = std::string{};

        for (hilet& token : _tokens) {
            if (auto string_ = std::get_if<string_type>(&token)) {
                r += *string_;
            } else if (std::holds_alternative<separator_type>(token)) {
                r += '/';
            } else if (std::holds_alternative<any_directory_type>(token)) {
                r += "/";
                return r;
            } else {
                return r;
            }
        }
        return r;
    }

    [[nodiscard]] std::filesystem::path base_path() const noexcept
    {
        return std::filesystem::path{base_generic_string()};
    }

    [[nodiscard]] bool matches(std::string_view path) const noexcept
    {
        return std::regex_match(path.begin(), path.end(), _regex);
    }

    [[nodiscard]] bool matches(std::string const& path) const noexcept
    {
        return matches(std::string_view{path});
    }

    [[nodiscard]] bool matches(char const *path) const noexcept
    {
        return matches(std::string_view{path});
    }

    [[nodiscard]] bool matches(std::filesystem::path const& path) const noexcept
    {
        return matches(path.generic_string());
    }

private:
    using string_type = std::string;
    struct separator_type {};
    using strings_type = std::vector<std::string>;
    struct any_string_type {};
    struct any_char_type {};
    struct any_directory_type {};

    using token_type =
        std::variant<string_type, separator_type, strings_type, any_string_type, any_char_type, any_directory_type>;

    using tokens_type = std::vector<token_type>;

    tokens_type _tokens;
    std::regex _regex;

    [[nodiscard]] constexpr static tokens_type parse(auto first, auto last)
    {
#define HI_GLOB_APPEND_STRING() \
    do { \
        if (not str.empty()) { \
            r.emplace_back(std::move(str)); \
            str.clear(); \
        } \
    } while (false)

        enum class state_type { idle, star, slash, slash_star, slash_star_star, bracket, brace };
        using enum state_type;

        static_assert(std::is_same_v<std::decay_t<decltype(*first)>, char>);

        auto r = tokens_type{};

        auto state = idle;
        auto str = string_type{};
        auto strs = strings_type{};

        auto it = first;
        while (it != last) {
            auto c = *it;
            switch (state) {
            case idle:
                switch (c) {
                case '/':
                    HI_GLOB_APPEND_STRING();
                    state = slash;
                    break;
                case '?':
                    HI_GLOB_APPEND_STRING();
                    r.emplace_back(any_char_type{});
                    break;
                case '*':
                    HI_GLOB_APPEND_STRING();
                    state = star;
                    break;
                case '[':
                    HI_GLOB_APPEND_STRING();
                    state = bracket;
                    break;
                case '{':
                    HI_GLOB_APPEND_STRING();
                    state = brace;
                    break;
                default:
                    str += c;
                }
                break;

            case star:
                if (c == '*') {
                    throw parse_error("Double /**/ is only allowed between slashes.");
                } else {
                    r.emplace_back(any_string_type{});
                    state = idle;
                    continue;
                }
                break;

            case slash:
                if (c == '*') {
                    state = slash_star;
                } else {
                    r.emplace_back(separator_type{});
                    state = idle;
                    continue;
                }
                break;

            case slash_star:
                if (c == '*') {
                    state = slash_star_star;
                } else {
                    r.emplace_back(separator_type{});
                    r.emplace_back(any_string_type{});
                    state = idle;
                    continue;
                }
                break;

            case slash_star_star:
                if (c == '/') {
                    r.emplace_back(any_directory_type{});
                    state = idle;
                } else {
                    throw parse_error(
                        std::format("'/**' must end in a slash in glob pattern '{}'", std::string_view{first, last}));
                }
                break;

            case bracket:
                if (c == ']') {
                    r.emplace_back(std::move(strs));
                    strs.clear();
                    state = idle;

                } else {
                    strs.emplace_back(1, c);
                }
                break;

            case brace:
                if (c == '}') {
                    if (not str.empty()) {
                        strs.push_back(std::move(str));
                        str.clear();
                    }
                    r.emplace_back(std::move(strs));
                    strs.clear();
                    state = idle;

                } else if (c == ',') {
                    strs.push_back(std::move(str));
                    str.clear();

                } else {
                    str += c;
                }
                break;

            default:
                hi_no_default();
            }

            ++it;
        }

        switch (state) {
        case idle:
            HI_GLOB_APPEND_STRING();
            break;

        case star:
            r.emplace_back(any_string_type{});
            break;

        case slash:
            r.emplace_back(separator_type{});
            break;

        case slash_star:
            r.emplace_back(separator_type{});
            r.emplace_back(any_string_type{});
            break;

        case slash_star_star:
            r.emplace_back(any_directory_type{});
            break;

        case bracket:
            throw parse_error("Unclosed bracket '[' found in glob pattern.");

        case brace:
            throw parse_error("Unclosed brace '{' found in glob pattern.");
        }

        return r;

#undef HI_GLOB_APPEND_STRING
    }

    [[nodiscard]] constexpr static tokens_type parse(auto&& range)
    {
        return parse(std::ranges::begin(range), std::ranges::end(range));
    }

    [[nodiscard]] constexpr static std::string make_regex_string(tokens_type const& tokens) noexcept
    {
        auto r = std::string{};

        for (hilet& token : tokens) {
            if (auto string_ = std::get_if<string_type>(&token)) {
                r += *string_;
            } else if (std::holds_alternative<separator_type>(token)) {
                r += '/';
            } else if (auto strings_ = std::get_if<strings_type>(&token)) {
                r += '(';
                auto first = true;
                for (hilet& s : *strings_) {
                    if (not std::exchange(first, false)) {
                        r += '|';
                    }
                    r += s;
                }
                r += ')';
            } else if (std::holds_alternative<any_char_type>(token)) {
                r += "[^/]";
            } else if (std::holds_alternative<any_string_type>(token)) {
                r += "[^/]*";
            } else if (std::holds_alternative<any_directory_type>(token)) {
                r += "(/.*)?/";
            } else {
                hi_no_default();
            }
        }
        return r;
    }

    [[nodiscard]] static std::regex make_regex(tokens_type const& tokens) noexcept
    {
        return std::regex{make_regex_string(tokens), std::regex_constants::ECMAScript | std::regex_constants::optimize};
    }
};

[[nodiscard]] inline generator<std::filesystem::path> glob(glob_pattern const& pattern)
{
    auto path = pattern.base_path();

    hilet first = std::filesystem::recursive_directory_iterator(path);
    hilet last = std::filesystem::recursive_directory_iterator();
    for (auto it = first; it != last; ++it) {
        if (pattern.matches(it->path())) {
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
