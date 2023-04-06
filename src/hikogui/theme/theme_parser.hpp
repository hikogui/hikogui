// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../parser/module.hpp"
#include "../color/module.hpp"

namespace hi { inline namespace v1 { namespace detail {

struct theme_length {
    float value;
    bool scales;

    [[nodiscard]] constexpr static theme_length pt(float x)
    {
        return theme_length{x, true};
    }

    [[nodiscard]] constexpr static theme_length px(float x)
    {
        return theme_length{x, false};
    }
};

struct theme_lengths : std::vector<theme_length> {};
struct theme_color_layers : std::vector<color> {};
struct theme_value : std::variant<theme_lengths, theme_length, theme_colors, color> {};

struct theme_pattern {
    std::vector<std::string> path;
    std::vector<bool> is_child;
    std::vector<std::string> states;
};

struct theme_selector : std::vector<theme_pattern> {};

struct theme_declaration {
    std::string name;
    theme_value value;
};

struct theme_rule_set {
    theme_selector selector;
    std::vector<theme_declaration> declarations;
};

struct theme_style_sheet {
    std::string name;
    std::string mode;

    std::vector<theme_rule_set> rule_sets;

};

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_selector> parse_theme_pattern(It& it, ItEnd last, std::filesystem::path const& path)
{
    // pattern := ( id | '*' ) ( '>'? ( id | '*' ) )*  ( ':' id )*
    auto r = theme_pattern{};

    if (it != last and *it == '*') {
        r.path.push_back("*");
        ++it;
    } else if (it != last and *it == token::id) {
        r.path.push_back(static_cast<std::string_view>(*it));
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
            r.is_child.push_back(is_child);
            r.path.push_back("*");
            is_child = false;
            ++it;

        } else if (*it == token::id) {
            r.is_child.push_back(is_child);
            r.path.push_back(static_cast<std::string_view>(*it));
            is_child = false;
            ++it;

        } else {
            throw parse_error(std::format(
                "{} Expecting element, '*', '>', ',' or '{' while parsing selector.", token_location(it, last, path)));
        }
    }

    while (it != last and *it == ':') {
        ++it;
        if (*it == node::id) {
            r.state.push_back(static_cast<std::string_view>(*it));
            ++it;

        } else {
            throw parse_error(std::format("{} Expecting state-id after ':' in selector.", token_location(it, last, path)));
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
        r.push_back(pattern);
    } else {
        return std::nullopt;
    }

    while (it != last and *it == ',') {
        ++it;

        if (auto pattern = parse_theme_pattern(it, last, path)) {
            r.push_back(pattern);
        } else {
            throw parse_error(std::format("{} Missing pattern after ',' in selector.", token_location(it, last, path)));
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<float> parse_theme_color_component(It& it, ItEnd last, std::filesystem::path const& path)
{
    auto r = 0.0f;

    if (it.size() >= 2 and (it[0] == token::integer or it[0] == token::real) and it[1] == '%') {
        r = static_cast<float>(it[0]) * 0.01;
        it += 2;
    } else if (it != last and *it == token::real) {
        r = static_cast<float>(*it);
        ++it;
    } else if (it != last and *it == token::integer) {
        r = sRGB_gamma_to_linear(static_cast<float>(*it) * 255.0f);
        ++it;
    } else {
        return std::nullopt;
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<float> parse_theme_alpha_component(It& it, ItEnd last, std::filesystem::path const& path)
{
    auto r = 0.0f;

    if (it.size() >= 2 and (it[0] == token::integer or it[0] == token::real) and it[1] == '%') {
        r = static_cast<float>(it[0]) * 0.01;
        it += 2;
    } else if (it != last and *it == token::real) {
        r = static_cast<float>(*it);
        ++it;
    } else {
        return std::nullopt;
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<color> parse_theme_color(It& it, ItEnd last, std::filesystem::path const& path)
{
    if (it != last and *it == token::color) {
        return static_cast<color>(*it);

    } else if (it != last and *it == token::id and *it == "rgb") {
        // rgb-color := "rgb" '(' color-component ','? color-component ','? color-component ( [,/]? alpha-component )? ')'
        // color-component := integer | float | number '%'
        // alpha-component := float | number '%'
        ++it;

        if (it != last and *it == '(') {
            ++it;
        } else {
            throw parse_error(std::format("{} Expect '(' after \"color-layers\" keyword.", token_location(it, last, path)));
        }

        auto r = color{};

        if (auto component = parse_theme_color_component(it, last, path)) {
            r.r() = *component;
        } else {
            throw parse_error(std::format("{} Expect a red-color-component after '('.", token_location(it, last, path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }

        if (auto component = parse_theme_color_component(it, last, path)) {
            r.g() = *component;
        } else {
            throw parse_error(std::format("{} Expect a green-color-component after '('.", token_location(it, last, path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }

        if (auto component = parse_theme_color_component(it, last, path)) {
            r.b() = *component;
        } else {
            throw parse_error(std::format("{} Expect a blue-color-component after '('.", token_location(it, last, path)));
        }

        if (it != last and (*it == ',' or *it == '/')) {
            ++it;
        }

        // Alpha is optional.
        if (auto component = parse_theme_alpha_component(it, last, path)) {
            r.a() = *component;
        }

        if (it != last and *it == ')') {
            ++it;
        } else {
            throw parse_error(std::format("{} Expect ')' after colors.", token_location(it, last, path)));
        }

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_color_layers>
parse_theme_color_layers(It& it, ItEnd last, std::filesystem::path const& path)
{
    if (it != last and *it == token::id and *it == "color-layers") {
        ++it;
    } else {
        return std::nullopt;
    }

    if (it != last and *it == '(') {
        ++it;
    } else {
        throw parse_error(std::format("{} Expect '(' after \"color-layers\" keyword.", token_location(it, last, path)));
    }

    auto r = theme_color_layers{};

    if (auto color = parse_theme_color(it, last, path)) {
        r.push_back(*color);
    } else {
        throw parse_error(std::format("{} Expect a color after '('.", token_location(it, last, path)));
    }

    while (it != last and *it == ',') {
        ++it;

        if (auto color = parse_theme_color(it, last, path)) {
            r.push_back(*color);
        } else {
            throw parse_error(std::format("{} Expect a color after ','.", token_location(it, last, path)));
        }
    }

    if (it != last and *it == ')') {
        ++it;
    } else {
        throw parse_error(std::format("{} Expect ')' after colors.", token_location(it, last, path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_length> parse_theme_length(It& it, ItEnd last, std::filesystem::path const& path)
{
    if (it.size() >= 2 and (it[0] == token::integer or it[0] == token::real) and it[1] == token::id) {
        if (it[1] == "pt") {
            return theme_length::pt(static_cast<float>(it[0]));
        } else if (it[1] == "px") {
            return theme_length::px(static_cast<float>(it[0]));
        } else {
            throw parse_error(std::format("{} Expected either \"pt\" ot \"px\" after number", token_location(it, last, path)));
        }

    } else if (it != last and (*it == token::integer or *it == token::real)) {
        return theme_length::pt(static_cast<float>(*it));

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_lengths> parse_theme_lengths(It& it, ItEnd last, std::filesystem::path const& path)
{
    // lengths := length length length*
    // length := number | number id
    if (it.size() < 3) {
        // Lengths are at least two numbers followed by semicolon.
        return std::nullopt;
    } else if (it[0] != token::integer and it[0] != token::real) {
        // The first element is always a number.
        return std::nullopt;
    } else if (it[1] == token::id and it[2] != token::integer and it[2] != token::real) {
        // If the first element is followed by an id, then the third element is always a number.
        return std::nullopt;
    } else if (it[1] != token::integer and it[1] != token::real) {
        // Otherwise the second element is always a number.
        return std::nullopt;
    }

    auto r = theme_lengths{};

    while (it != last) {
        if (auto length = parse_theme_length(it, last, path)) {
            r.push_back(*length);
        } else {
            break;
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_value> parse_theme_value(It& it, ItEnd last, std::filesystem::path const& path)
{
    if (auto color_layers = parse_theme_color_layers(it, last, path)) {
        return theme_value{*color_layers};

    } else if (auto color = parse_theme_color(it, last, path)) {
        return theme_value{*color};

    } else if (auto lengths = parse_theme_lengths(it, last, path)) {
        return theme_value{*lengths};

    } else if (auto length = parse_theme_length(it, last, path)) {
        return theme_value{*length};

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_declaration>
parse_theme_declaration(It& it, ItEnd last, std::filesystem::path const& path)
{
    // declaration := id ':' value ';'
    auto r = theme_declaration{};

    if (it != last and *it == token::id) {
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
            r.declarations.push_back(*declaration);
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

}}} // namespace hi::v1::detail
