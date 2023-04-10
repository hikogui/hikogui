// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../parser/module.hpp"
#include "../color/module.hpp"

namespace hi { inline namespace v1 { namespace detail {

struct theme_length {
    enum class length_type { px, pt, em };

    float value;
    length_type type;

    [[nodiscard]] constexpr static theme_length pt(float x)
    {
        return theme_length{x, length_type::pt};
    }

    [[nodiscard]] constexpr static theme_length in(float x)
    {
        return pt(x * 72.0f);
    }

    [[nodiscard]] constexpr static theme_length cm(float x)
    {
        return pt(x * 28.3464567f);
    }

    [[nodiscard]] constexpr static theme_length px(float x)
    {
        return theme_length{x, length_type::px};
    }

    [[nodiscard]] constexpr static theme_length em(float x)
    {
        return theme_length{x, length_type::em};
    }
};

struct theme_color {
    enum class color_type {
        none,
        custom,
        black,
        silver,
        gray,
        white,
        maroon,
        red,
        purple,
        fuchsia,
        green,
        line,
        olive,
        yellow,
        navy,
        blue,
        teal,
        aqua,
        indigo,
        orange,
        pink,
        background,
        gray1,
        gray2,
        gray3,
        gray4,
        gray5,
        gray6,
        gray7,
        gray8,
        gray9,
        foreground
    };

    // clang-format off
    constexpr static auto color_type_metadata = enum_metadata{
        color_type::none, "x-none",
        color_type::custom,  "x-custom",
        color_type::black, "black",
        color_type::silver, "silver",
        color_type::gray, "gray",
        color_type::white, "white",
        color_type::maroon, "maroon",
        color_type::red, "red",
        color_type::purple, "purple",
        color_type::fuchsia, "fuchsia",
        color_type::green, "green",
        color_type::line, "line",
        color_type::olive, "olive",
        color_type::yellow, "yellow",
        color_type::navy, "navy",
        color_type::blue, "blue",
        color_type::teal, "teal",
        color_type::aqua, "aqua",
        color_type::indigo, "indigo",
        color_type::orange, "orange",
        color_type::pink, "pink",
        color_type::background, "background",
        color_type::gray1, "gray1",
        color_type::gray2, "gray2",
        color_type::gray3, "gray3",
        color_type::gray4, "gray4",
        color_type::gray5, "gray5",
        color_type::gray6, "gray6",
        color_type::gray7, "gray7",
        color_type::gray8, "gray8",
        color_type::gray9, "gray9",
        color_type::foreground, "foreground",
    };
    // clang-format on

    color value;
    color_type type;

    constexpr theme_color(color value) noexcept : value(value), type(color_type::custom) {}
    constexpr theme_color(color_type type) noexcept : value(), type(type) {}
    constexpr theme_color(std::string_view name) noexcept : value(), type(color_type_metadata.at(name, color_type::none)) {}
};

struct theme_value : std::variant<theme_length, theme_color, std::string> {};

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
[[nodiscard]] constexpr std::optional<theme_color> parse_theme_color(It& it, ItEnd last, std::filesystem::path const& path)
{
    if (it != last and *it == token::color) {
        return theme_color{static_cast<color>(*it)};

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

        auto rgba = color{};

        if (auto component = parse_theme_color_component(it, last, path)) {
            rgba.r() = *component;
        } else {
            throw parse_error(std::format("{} Expect a red-color-component after '('.", token_location(it, last, path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }

        if (auto component = parse_theme_color_component(it, last, path)) {
            rgba.g() = *component;
        } else {
            throw parse_error(std::format("{} Expect a green-color-component after '('.", token_location(it, last, path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }

        if (auto component = parse_theme_color_component(it, last, path)) {
            rgba.b() = *component;
        } else {
            throw parse_error(std::format("{} Expect a blue-color-component after '('.", token_location(it, last, path)));
        }

        if (it != last and (*it == ',' or *it == '/')) {
            ++it;
        }

        // Alpha is optional.
        if (auto component = parse_theme_alpha_component(it, last, path)) {
            rgba.a() = *component;
        }

        if (it != last and *it == ')') {
            ++it;
        } else {
            throw parse_error(std::format("{} Expect ')' after colors.", token_location(it, last, path)));
        }

        return theme_color{rgba};

    } else if (it != last and *it == token::id) {
        // A color name.
        if (auto r = theme_color{static_cast<std::string_view>(*it)}; not r.empty()) {
            ++it;
            return r;
        } else {
            return std::nullopt;
        }

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<color> parse_theme_colors(It& it, ItEnd last, std::filesystem::path const& path)
{
    auto r = std::vector<color>{};

    if (auto color = parse_theme_color(it, last, path)) {
        r.push_back(*color);
    } else {
        return r;
    }

    if (it != last and *it == ',') {
        ++it;
    }

    while (it != last and *it != ';') {
        if (auto color = parse_theme_color(it, last, path)) {
            r.push_back(*color);
        } else {
            throw parse_error(std::format("{} Expect a sequence of colors.", token_location(it, last, path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_length> parse_theme_length(It& it, ItEnd last, std::filesystem::path const& path)
{
    if (it.size() >= 2 and (it[0] == token::integer or it[0] == token::real) and it[1] == token::id) {
        if (it[1] == "pt") {
            return theme_length::pt(static_cast<float>(it[0]));
        } else if (it[1] == "cm") {
            return theme_length::cm(static_cast<float>(it[0]));
        } else if (it[1] == "in") {
            return theme_length::in(static_cast<float>(it[0]));
        } else if (it[1] == "px") {
            return theme_length::px(static_cast<float>(it[0]));
        } else if (it[1] == "em") {
            return theme_length::em(static_cast<float>(it[0]));
        } else {
            throw parse_error(std::format(
                "{} Expected either \"pt\", \"cm\", \"in\", \"em\" or \"px\" after number", token_location(it, last, path)));
        }

    } else if (it != last and (*it == token::integer or *it == token::real)) {
        // Implicitly a number without suffix is in `pt`.
        return theme_length::pt(static_cast<float>(*it));

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_length> parse_theme_lengths(It& it, ItEnd last, std::filesystem::path const& path)
{
    auto r = std::vector<theme_length>{};

    if (auto length = parse_theme_length(it, last, path)) {
        r.push_back(*length);
    } else {
        return r;
    }

    if (it != last and *it == ',') {
        ++it;
    }

    while (it != last and *it != ';') {
        if (auto length = parse_theme_length(it, last, path)) {
            r.push_back(*length);
        } else {
            throw parse_error(std::format("{} Expect a sequence of lengths.", token_location(it, last, path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_value> parse_theme_value(It& it, ItEnd last, std::filesystem::path const& path)
{
    if (auto color = parse_theme_color(it, last, path)) {
        return theme_value{*color};

    } else if (auto length = parse_theme_length(it, last, path)) {
        return theme_value{*length};

    } else if (it != last and *it == token::string) {
        auto r = theme_value{static_cast<std::string>(*it);
        ++it;
        return r;

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_margin_declarations(It& it, ItEnd last, std::filesystem::path const& path)
{
    auto r = std::vector<theme_declaration>{};

    if (auto lengths = parse_theme_lengths(it, last, path); not lengths.empty()) {
        if (lengths.size() == 1) {
            r.emplace_back("margin-top", lengths[0]);
            r.emplace_back("margin-right", lengths[0]);
            r.emplace_back("margin-bottom", lengths[0]);
            r.emplace_back("margin-left", lengths[0]);
        } else if (lengths.size() == 2) {
            r.emplace_back("margin-top", lengths[0]);
            r.emplace_back("margin-right", lengths[1]);
            r.emplace_back("margin-bottom", lengths[0]);
            r.emplace_back("margin-left", lengths[1]);
        } else if (lengths.size() == 3) {
            r.emplace_back("margin-top", lengths[0]);
            r.emplace_back("margin-right", lengths[1]);
            r.emplace_back("margin-bottom", lengths[0]);
            r.emplace_back("margin-left", lengths[2]);
        } else if (lengths.size() == 4) {
            r.emplace_back("margin-top", lengths[0]);
            r.emplace_back("margin-right", lengths[1]);
            r.emplace_back("margin-bottom", lengths[2]);
            r.emplace_back("margin-left", lengths[3]);
        } else {
            throw parse_error(std::format(
                "{} Expect 1 to 4 length values when parsing \"margin\" declaration, got {}.",
                token_location(it, last, path),
                lengths.size()));
        }
    } else {
        throw parse_error(
            std::format("{} Expect 1 to 4 length values when parsing \"margin\" declaration.", token_location(it, last, path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_spacing_declarations(It& it, ItEnd last, std::filesystem::path const& path)
{
    auto r = std::vector<theme_declaration>{};

    if (auto lengths = parse_theme_lengths(it, last, path); not lengths.empty()) {
        if (lengths.size() == 1) {
            r.emplace_back("spacing-vertical", lengths[0]);
            r.emplace_back("spacing-horizontal", lengths[0]);
        } else if (lengths.size() == 2) {
            r.emplace_back("spacing-vertical", lengths[0]);
            r.emplace_back("spacing-horizontal", lengths[1]);
        } else {
            throw parse_error(std::format(
                "{} Expect 1 or 2 length values when parsing \"spacing\" declaration, got {}.",
                token_location(it, last, path),
                lengths.size()));
        }
    } else {
        throw parse_error(
            std::format("{} Expect 1 or 2 length values when parsing \"spacing\" declaration.", token_location(it, last, path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_border_radius_declarations(It& it, ItEnd last, std::filesystem::path const& path)
{
    auto r = std::vector<theme_declaration>{};

    if (auto lengths = parse_theme_lengths(it, last, path); not lengths.empty()) {
        if (lengths.size() == 1) {
            r.emplace_back("border-top-left-radius", lengths[0]);
            r.emplace_back("border-top-right-radius", lengths[0]);
            r.emplace_back("border-bottom-left-radius", lengths[0]);
            r.emplace_back("border-bottom-right-radius", lengths[0]);
        } else if (lengths.size() == 2) {
            r.emplace_back("border-top-left-radius", lengths[0]);
            r.emplace_back("border-top-right-radius", lengths[1]);
            r.emplace_back("border-bottom-left-radius", lengths[1]);
            r.emplace_back("border-bottom-right-radius", lengths[0]);
        } else if (lengths.size() == 4) {
            r.emplace_back("border-top-left-radius", lengths[0]);
            r.emplace_back("border-top-right-radius", lengths[1]);
            r.emplace_back("border-bottom-left-radius", lengths[2]);
            r.emplace_back("border-bottom-right-radius", lengths[3]);
        } else {
            throw parse_error(std::format(
                "{} Expect 1, 2 or 4 length values when parsing \"border-radius\" declaration, got {}.",
                token_location(it, last, path),
                lengths.size()));
        }
    } else {
        throw parse_error(std::format(
            "{} Expect 1, 2 or 4 length values when parsing \"border-radius\" declaration.", token_location(it, last, path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_caret_color_declarations(It& it, ItEnd last, std::filesystem::path const& path)
{
    auto r = std::vector<theme_declaration>{};

    if (auto colors = parse_theme_colors(it, last, path); not colors.empty()) {
        if (colors.size() == 1) {
            r.emplace_back("caret-color-primary", colors[0]);
            r.emplace_back("caret-color-secondary", colors[0]);
        } else if (lengths.size() == 2) {
            r.emplace_back("caret-color-primary", colors[0]);
            r.emplace_back("caret-color-secondary", colors[1]);
        } else {
            throw parse_error(std::format(
                "{} Expect 1 or 2 color values when parsing \"caret-color\" declaration, got {}.",
                token_location(it, last, path),
                lengths.size()));
        }
    } else {
        throw parse_error(std::format(
            "{} Expect 1 or 2 color values when parsing \"caret-color\" declaration.", token_location(it, last, path)));
    }

    return r;
}
template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_declaration(It& it, ItEnd last, std::filesystem::path const& path)
{
    // declaration := id ':' value ';'
    auto r = std::vector<theme_declaration>{};
    auto name = std::string{};

    if (it != last and *it == token::id) {
        name = static_cast<std::string_view>(*it);
        ++it;
    } else {
        return std::nullopt;
    }

    if (it != last and *it == ':') {
        ++it;
    } else {
        throw parse_error(std::format("{} Missing ':' while parsing declaration.", token_location(it, last, path)));
    }

    if (name == "margin") {
        r = parse_theme_margin_declarations(it, last, path);

    } else if (name == "spacing") {
        r = parse_theme_spacing_declarations(it, last, path);

    } else if (name == "border-radius") {
        r = parse_theme_border_radius_declarations(it, last, path);

    } else if (name == "caret-color") {
        r = parse_theme_caret_color_declarations(it, last, path);

    } else if (name == "font-type") {
        r = parse_theme_font_type(it, last, path);

    } else if (name == "font-weight") {
        r = parse_theme_font_type(it, last, path);

    } else {
        // Other names.
        if (auto value = parse_theme_value(it, last, path)) {
            r.emplace_back(name, value);

        } else {
            throw parse_error(
                std::format("{} Missing value after ':' while parsing declaration.", token_location(it, last, path)));
        }
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
