// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../parser/module.hpp"

namespace hi { inline namespace v1 {

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_selector> parse_theme_pattern(It& it, ItEnd last, std::filesystem::path const& path)
{
    // pattern := ( id | '*' ) ( '>'? ( id | '*' ) )*  ( ':' id )*
    auto r = theme_pattern{};

    hi_assert(it != last);

    if (*it == '*') {
        r.set_first("*");
        ++it;
    } else if (*it == token::id) {
        r.set_first(static_cast<std::string_view>(*it));
        ++it;
    } else {
        return std::nullopt;
    }

    auto id_child = false;
    while (it != last and *it != ',' and *it != '{' and *it != ':') {
        if (*it == '>') {
            is_child = true;
            ++it;

        } else if (*it == '*') {
            if (is_child) {
                r.add_child("*");
            } else {
                r.add_descended("*");
            }
            is_child = true;
            ++it;

        } else if (*it == token::id) {
            if (is_child) {
                r.add_child(static_cast<std::string_view>(*it));
            } else {
                r.add_descended(static_cast<std::string_view>(*it));
            }
            is_child = true;
            ++it;

        } else {
            throw parse_error(
                std::format("{} Expecting element, '*', '>', ',' or '{' while parsing selector.", token_location(it, last, path)));
        }
    }

    while (it != last and *it == ':') {
        ++it;
        if (*it == node::id) {
            r.add_state(static_cast<std::string_view>(*it));
            ++it;

        } else {
            throw parse_error(
                std::format("{} Expecting state-id after ':' in selector.", token_location(it, last, path)));
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_selector> parse_theme_selector(It& it, ItEnd last, std::filesystem::path const& path)
{
    // selector := pattern (',' pattern)*
    auto r = theme_selector{};

    if (auto pattern = parse_theme_pattern(it, last, path)) {
        r.add(pattern);
    } else {
        return std::nullopt;
    }

    while (it != last and *it == ',') {
        ++it;

        if (auto pattern = parse_theme_pattern(it, last, path)) {
            r.add(pattern);
        } else {
            throw parse_error(std::format("{} Missing pattern after ',' in selector.", token_location(it, last, path)));
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_value>
parse_theme_value(It& it, ItEnd last, std::filesystem::path const& path)
{
    if (auto colors = parse_theme_
    if (auto lengths = parse_theme_lengths(it, last, path)) {
        return theme_value{*lengths};
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_declaration> parse_theme_declaration(It& it, ItEnd last, std::filesystem::path const& path)
{
    // declaration := id ':' value ';'
    auto r = theme_declaration{};

    hi_assert(it != last);

    if (*it == token::id) {
        r.name = static_cast<std::string_view>(*it);
        ++it;
    } else {
        return std::nullopt;
    }

    if (it != last and *it == ':') {
        ++it;
    } else {
        throw parse_error(std::format("{} Missing ':' while parsing declaration.", token_location(it, last, path)));
    }

    if (auto value = parse_theme_value(it, last, path)) {
        r.value = value;
    } else {
        throw parse_error(std::format("{} Missing value after ':' while parsing declaration.", token_location(it, last, path)));
    }

    if (it != last and *it == ';') {
        ++it;
    } else {
        throw parse_error(std::format("{} Missing ';' after value while parsing declaration.", token_location(it, last, path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_rule_set> parse_theme_rule_set(It& it, ItEnd last, std::filesystem::path const& path)
{
    // rule_set := selector '{' declaration* '}'
    auto r = theme_rule_set{};

    if (auto selector = parse_theme_selector(it, last, path)) {
        r.selector = *selector;
    } else {
        return std::nullopt;
    }

    if (it != last and *it == '{') {
        ++it;
    } else {
        throw parse_error(std::format("{} Missing '{{' while parsing rule-set.", token_location(it, last, path)));
    }

    while (it != last and *it != '}') {
        if (auto declaration = parse_theme_declaration(it, last, path)) {
            r.add(*declaration);
        } else {
            throw parse_error(std::format("{} Missing declaration while parsing rule-set.", token_location(it, last, path)));
        }
    }

    if (it != last and *it == '}') {
        ++it;
    } else {
        throw parse_error(std::format("{} Missing '}}' while parsing rule-set.", token_location(it, last, path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_stylesheet>
parse_theme_stylesheet(It& it, ItEnd last, std::filesystem::path const& path)
{
    // stylesheet := ( at_rule | rule_set )*
    auto r = theme_stylesheet{};

    while (it != last) {
        if (auto at_rule = parse_theme_at_rule(it, last, path)) {
            r.add(*at_rule);
        } else if (auto rule_set = parse_theme_rule_set(it, last, path)) {
            r.add(*rule_set);
        }
    }

    return r;
}
}} // namespace hi::v1
