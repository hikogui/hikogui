// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../parser/module.hpp"
#include "../color/module.hpp"
#include "../font/module.hpp"
#include <string>
#include <vector>

namespace hi { inline namespace v1 {
namespace detail {

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

struct theme_value : std::variant<theme_length, hi::color, font_family_id, font_weight, font_style> {};

struct theme_pattern {
    std::vector<std::string> path;
    std::vector<bool> is_child;
    std::vector<std::string> states;
};

struct theme_selector : std::vector<theme_pattern> {};

struct theme_declaration {
    std::string name;
    theme_value value;

    constexpr theme_declaration(std::string name, theme_value value) noexcept : name(std::move(name)), value(value) {}
    constexpr theme_declaration(std::string name, theme_length length) noexcept : name(std::move(name)), value({length}) {}
    constexpr theme_declaration(std::string name, hi::color color) noexcept : name(std::move(name)), value({color}) {}
    constexpr theme_declaration(std::string name, font_family_id id) noexcept : name(std::move(name)), value({id}) {}
    constexpr theme_declaration(std::string name, font_style style) noexcept : name(std::move(name)), value({style}) {}
    constexpr theme_declaration(std::string name, font_weight weight) noexcept : name(std::move(name)), value({weight}) {}
};

struct theme_rule_set {
    theme_selector selector;
    std::vector<theme_declaration> declarations;
};

} // namespace detail

struct theme_style_sheet {
    std::string name;
    std::string mode;

    std::vector<std::pair<std::string, color>> colors;
    std::vector<detail::theme_rule_set> rule_sets;
};

namespace detail {

class parse_theme_context {
public:
    std::filesystem::path path;

    [[nodiscard]] constexpr bool set_macro(std::string const& name, std::vector<theme_declaration> declarations) noexcept
    {
        auto it = std::lower_bound(_macros.begin(), _macros.end(), name, [](hilet& item, hilet& value) {
            return item.first < value;
        });
        if (it != _macros.end() and it->first == name) {
            return false;
        } else {
            _macros.emplace(it, name, std::move(declarations));
            return true;
        }
    }

    constexpr std::optional<std::vector<theme_declaration>> get_macro(std::string const& name) const noexcept
    {
        auto it = std::lower_bound(_macros.begin(), _macros.end(), name, [](hilet& item, hilet& value) {
            return item.first < value;
        });
        if (it != _macros.end() and it->first == name) {
            return it->second;
        } else {
            return std::nullopt;
        }
    }

    [[nodiscard]] constexpr bool set_let(std::string const& name, theme_value value) noexcept
    {
        auto it = std::lower_bound(_lets.begin(), _lets.end(), name, [](hilet& item, hilet& value) {
            return item.first < value;
        });
        if (it != _lets.end() and it->first == name) {
            return false;
        } else {
            _lets.emplace(it, name, value);
            return true;
        }
    }

    constexpr std::optional<theme_value> get_let(std::string const& name) const noexcept
    {
        auto it = std::lower_bound(_lets.begin(), _lets.end(), name, [](hilet& item, hilet& value) {
            return item.first < value;
        });
        if (it != _lets.end() and it->first == name) {
            return it->second;
        } else {
            return std::nullopt;
        }
    }

    [[nodiscard]] constexpr bool set_color(std::string const& name, hi::color color) noexcept
    {
        auto it = std::lower_bound(_colors.begin(), _colors.end(), name, [](hilet& item, hilet& value) {
            return item.first < value;
        });
        if (it != _colors.end() and it->first == name) {
            return false;
        } else {
            _colors.emplace(it, name, color);
            return true;
        }
    }

    constexpr std::optional<color> get_color(std::string const& name) const noexcept
    {
        auto it = std::lower_bound(_colors.begin(), _colors.end(), name, [](hilet& item, hilet& value) {
            return item.first < value;
        });
        if (it != _colors.end() and it->first == name) {
            return it->second;
        } else {
            return std::nullopt;
        }
    }

    constexpr std::vector<std::pair<std::string, color>> move_colors() noexcept
    {
        return std::move(_colors);
    }

private:
    std::vector<std::pair<std::string, color>> _colors;
    std::vector<std::pair<std::string, std::vector<theme_declaration>>> _macros;
    std::vector<std::pair<std::string, theme_value>> _lets;
};

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_pattern> parse_theme_pattern(It& it, ItEnd last, parse_theme_context& context)
{
    // pattern := ( id | '*' ) ( '>'? ( id | '*' ) )*  ( ':' id )*
    auto r = theme_pattern{};

    if (it != last and *it == '*') {
        r.path.push_back("*");
        ++it;
    } else if (it != last and *it == token::id) {
        r.path.push_back(static_cast<std::string>(*it));
        ++it;
    } else {
        return std::nullopt;
    }

    auto is_child = false;
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
            r.path.push_back(static_cast<std::string>(*it));
            is_child = false;
            ++it;

        } else {
            throw parse_error(std::format(
                "{} Expecting element, '*', '>', ',' or '{{' while parsing selector.", token_location(it, last, context.path)));
        }
    }

    while (it != last and *it == ':') {
        ++it;
        if (*it == token::id) {
            r.states.push_back(static_cast<std::string>(*it));
            ++it;

        } else {
            throw parse_error(
                std::format("{} Expecting state-id after ':' in selector.", token_location(it, last, context.path)));
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_selector> parse_theme_selector(It& it, ItEnd last, parse_theme_context& context)
{
    // selector := pattern (',' pattern)*
    auto r = theme_selector{};

    if (auto pattern = parse_theme_pattern(it, last, context)) {
        r.push_back(*pattern);
    } else {
        return std::nullopt;
    }

    while (it != last and *it == ',') {
        ++it;

        if (auto pattern = parse_theme_pattern(it, last, context)) {
            r.push_back(*pattern);
        } else {
            throw parse_error(std::format("{} Missing pattern after ',' in selector.", token_location(it, last, context.path)));
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<float> parse_theme_color_component(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = 0.0f;

    if (it.size() >= 2 and (it[0] == token::integer or it[0] == token::real) and it[1] == '%') {
        r = static_cast<float>(it[0]) * 0.01f;
        it += 2;
    } else if (it != last and *it == token::real) {
        r = static_cast<float>(*it);
        ++it;
    } else if (it.size() >= 2 and it[0] == '-' and it[1] == token::real) {
        r = -static_cast<float>(it[1]);
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
[[nodiscard]] constexpr std::optional<float> parse_theme_alpha_component(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = 0.0f;

    if (it.size() >= 2 and (it[0] == token::integer or it[0] == token::real) and it[1] == '%') {
        r = static_cast<float>(it[0]) * 0.01f;
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
[[nodiscard]] constexpr std::optional<color> parse_theme_color(It& it, ItEnd last, parse_theme_context& context)
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
            throw parse_error(
                std::format("{} Expect '(' after \"color-layers\" keyword.", token_location(it, last, context.path)));
        }

        auto r = color{};

        if (auto component = parse_theme_color_component(it, last, context)) {
            r.r() = *component;
        } else {
            throw parse_error(std::format("{} Expect a red-color-component after '('.", token_location(it, last, context.path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }

        if (auto component = parse_theme_color_component(it, last, context)) {
            r.g() = *component;
        } else {
            throw parse_error(
                std::format("{} Expect a green-color-component after '('.", token_location(it, last, context.path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }

        if (auto component = parse_theme_color_component(it, last, context)) {
            r.b() = *component;
        } else {
            throw parse_error(std::format("{} Expect a blue-color-component after '('.", token_location(it, last, context.path)));
        }

        if (it != last and (*it == ',' or *it == '/')) {
            ++it;
        }

        // Alpha is optional.
        if (auto component = parse_theme_alpha_component(it, last, context)) {
            r.a() = *component;
        }

        if (it != last and *it == ')') {
            ++it;
        } else {
            throw parse_error(std::format("{} Expect ')' after colors.", token_location(it, last, context.path)));
        }

        return r;

    } else if (it != last and *it == token::id) {
        // A color name is looked up from @color declarations
        hilet name = static_cast<std::string>(*it);
        ++it;

        if (auto color = context.get_color(name)) {
            return *color;

        } else {
            throw parse_error(
                std::format("{} Color name \"{}\" was not declared with @color.", token_location(it, last, context.path), name));
        }

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<color> parse_theme_colors(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<color>{};

    if (auto color = parse_theme_color(it, last, context)) {
        r.push_back(*color);
    } else {
        return r;
    }

    if (it != last and *it == ',') {
        ++it;
    }

    while (it != last and *it != ';') {
        if (auto color = parse_theme_color(it, last, context)) {
            r.push_back(*color);
        } else {
            throw parse_error(std::format("{} Expect a sequence of colors.", token_location(it, last, context.path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_length> parse_theme_length(It& it, ItEnd last, parse_theme_context& context)
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
                "{} Expected either \"pt\", \"cm\", \"in\", \"em\" or \"px\" after number",
                token_location(it, last, context.path)));
        }

    } else if (it != last and (*it == token::integer or *it == token::real)) {
        // Implicitly a number without suffix is in `pt`.
        return theme_length::pt(static_cast<float>(*it));

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_length> parse_theme_lengths(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<theme_length>{};

    if (auto length = parse_theme_length(it, last, context)) {
        r.push_back(*length);
    } else {
        return r;
    }

    if (it != last and *it == ',') {
        ++it;
    }

    while (it != last and *it != ';') {
        if (auto length = parse_theme_length(it, last, context)) {
            r.push_back(*length);
        } else {
            throw parse_error(std::format("{} Expect a sequence of lengths.", token_location(it, last, context.path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_value> parse_theme_let_expansion(It& it, ItEnd last, parse_theme_context& context)
{
    if (it.size() < 2 or it[0] != '@' or it[1] != token::id) {
        return std::nullopt;
    }

    hilet name = static_cast<std::string>(it[1]);
    it += 2;

    if (auto value = context.get_let(name)) {
        return {value};
    } else {
        throw parse_error(std::format("{} Trying to expand undeclared @let {}.", token_location(it, last, context.path), name));
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_value> parse_theme_value(It& it, ItEnd last, parse_theme_context& context)
{
    if (auto value = parse_theme_let_expansion(it, last, context)) {
        return value;

    } else if (auto color = parse_theme_color(it, last, context)) {
        return theme_value{*color};

    } else if (auto length = parse_theme_length(it, last, context)) {
        return theme_value{*length};

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_font_family_declaration(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<theme_declaration>{};

    if (it != last and (*it == token::id or *it == token::dstr)) {
        auto family_name = static_cast<std::string>(*it);
        if (auto family_id = find_font_family(family_name)) {
            r.emplace_back("font-family", family_id);
        } else {
            throw parse_error(std::format(
                "{} Could not find font-family \"{}\" in the font-book.", token_location(it, last, context.path), family_name));
        }

    } else {
        throw parse_error(
            std::format("{} Expecting a string or name in font-family declaration.", token_location(it, last, context.path)));
    }

    ++it;
    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_font_style_declaration(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<theme_declaration>{};

    if (it != last and *it == token::id and *it == "normal") {
        r.emplace_back("font-style", font_style::normal);
    } else if (it != last and *it == token::id and *it == "italic") {
        r.emplace_back("font-style", font_style::italic);
    } else if (it != last and *it == token::id and *it == "oblique") {
        r.emplace_back("font-style", font_style::oblique);
    } else {
        throw parse_error(std::format(
            "{} Expecting normal, italic or oblique as value of a font-style declaration.",
            token_location(it, last, context.path)));
    }

    ++it;
    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_font_weight_declaration(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<theme_declaration>{};

    if (it != last and *it == token::id and *it == "thin") {
        r.emplace_back("font-weight", font_weight::thin);
    } else if (it != last and *it == token::id and *it == "extra-light") {
        r.emplace_back("font-weight", font_weight::extra_light);
    } else if (it != last and *it == token::id and *it == "light") {
        r.emplace_back("font-weight", font_weight::light);
    } else if (it != last and *it == token::id and (*it == "regular" or *it == "normal")) {
        r.emplace_back("font-weight", font_weight::regular);
    } else if (it != last and *it == token::id and *it == "medium") {
        r.emplace_back("font-weight", font_weight::medium);
    } else if (it != last and *it == token::id and *it == "semi-bold") {
        r.emplace_back("font-weight", font_weight::semi_bold);
    } else if (it != last and *it == token::id and *it == "bold") {
        r.emplace_back("font-weight", font_weight::bold);
    } else if (it != last and *it == token::id and *it == "extra-bold") {
        r.emplace_back("font-weight", font_weight::extra_bold);
    } else if (it != last and *it == token::id and *it == "black") {
        r.emplace_back("font-weight", font_weight::black);
    } else if (it != last and *it == token::id and *it == "extra-black") {
        r.emplace_back("font-weight", font_weight::extra_black);

    } else if (it != last and *it == token::integer) {
        r.emplace_back("font-weight", font_weight_from_int(static_cast<int>(*it)));

    } else {
        throw parse_error(std::format(
            "{} Expecting a integer or  value of a font-weight declaration.", token_location(it, last, context.path)));
    }

    ++it;
    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_margin_declarations(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<theme_declaration>{};

    if (auto lengths = parse_theme_lengths(it, last, context); not lengths.empty()) {
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
                token_location(it, last, context.path),
                lengths.size()));
        }
    } else {
        throw parse_error(std::format(
            "{} Expect 1 to 4 length values when parsing \"margin\" declaration.", token_location(it, last, context.path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_spacing_declarations(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<theme_declaration>{};

    if (auto lengths = parse_theme_lengths(it, last, context); not lengths.empty()) {
        if (lengths.size() == 1) {
            r.emplace_back("spacing-vertical", lengths[0]);
            r.emplace_back("spacing-horizontal", lengths[0]);
        } else if (lengths.size() == 2) {
            r.emplace_back("spacing-vertical", lengths[0]);
            r.emplace_back("spacing-horizontal", lengths[1]);
        } else {
            throw parse_error(std::format(
                "{} Expect 1 or 2 length values when parsing \"spacing\" declaration, got {}.",
                token_location(it, last, context.path),
                lengths.size()));
        }
    } else {
        throw parse_error(std::format(
            "{} Expect 1 or 2 length values when parsing \"spacing\" declaration.", token_location(it, last, context.path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_border_radius_declarations(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<theme_declaration>{};

    if (auto lengths = parse_theme_lengths(it, last, context); not lengths.empty()) {
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
                token_location(it, last, context.path),
                lengths.size()));
        }
    } else {
        throw parse_error(std::format(
            "{} Expect 1, 2 or 4 length values when parsing \"border-radius\" declaration.",
            token_location(it, last, context.path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_declaration>
parse_theme_caret_color_declarations(It& it, ItEnd last, parse_theme_context& context)
{
    auto r = std::vector<theme_declaration>{};

    if (auto colors = parse_theme_colors(it, last, context); not colors.empty()) {
        if (colors.size() == 1) {
            r.emplace_back("caret-color-primary", colors[0]);
            r.emplace_back("caret-color-secondary", colors[0]);
        } else if (colors.size() == 2) {
            r.emplace_back("caret-color-primary", colors[0]);
            r.emplace_back("caret-color-secondary", colors[1]);
        } else {
            throw parse_error(std::format(
                "{} Expect 1 or 2 color values when parsing \"caret-color\" declaration, got {}.",
                token_location(it, last, context.path),
                colors.size()));
        }
    } else {
        throw parse_error(std::format(
            "{} Expect 1 or 2 color values when parsing \"caret-color\" declaration.", token_location(it, last, context.path)));
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<std::vector<theme_declaration>>
parse_theme_macro_expansion(It& it, ItEnd last, parse_theme_context& context)
{
    if (it.size() < 2 or it[0] != '@' or it[1] != token::id) {
        return std::nullopt;
    }

    hilet name = static_cast<std::string>(it[1]);
    it += 2;

    auto r = std::vector<theme_declaration>{};
    if (auto declarations = context.get_macro(name)) {
        r = std::move(*declarations);
    } else {
        throw parse_error(std::format("{} Trying to expand undeclared @macro {}.", token_location(it, last, context.path), name));
    }

    if (it == last or *it != ';') {
        throw parse_error(std::format(
            "{} Missing ';' after @macro {} expansion while parsing declaration.", token_location(it, last, context.path), name));
    }

    return {std::move(r)};
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<std::vector<theme_declaration>>
parse_theme_declaration(It& it, ItEnd last, parse_theme_context& context)
{
    // declaration := id ':' value ';'
    auto r = std::vector<theme_declaration>{};

    if (it.size() < 2 or it[0] != token::id or it[1] != ':') {
        return std::nullopt;
    }

    hilet name = static_cast<std::string>(*it);
    it += 2;

    if (name == "margin") {
        r = parse_theme_margin_declarations(it, last, context);

    } else if (name == "spacing") {
        r = parse_theme_spacing_declarations(it, last, context);

    } else if (name == "border-radius") {
        r = parse_theme_border_radius_declarations(it, last, context);

    } else if (name == "caret-color") {
        r = parse_theme_caret_color_declarations(it, last, context);

    } else if (name == "font-family") {
        r = parse_theme_font_family_declaration(it, last, context);

    } else if (name == "font-style") {
        r = parse_theme_font_style_declaration(it, last, context);

    } else if (name == "font-weight") {
        r = parse_theme_font_weight_declaration(it, last, context);

    } else {
        // Other names.
        if (auto value = parse_theme_value(it, last, context)) {
            r.emplace_back(name, *value);

        } else {
            throw parse_error(
                std::format("{} Missing value after ':' while parsing declaration.", token_location(it, last, context.path)));
        }
    }

    if (it == last or *it != ';') {
        throw parse_error(
            std::format("{} Missing ';' after value while parsing declaration.", token_location(it, last, context.path)));
    }

    ++it;
    return {std::move(r)};
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_rule_set> parse_theme_rule_set(It& it, ItEnd last, parse_theme_context& context)
{
    // rule_set := selector '{' declaration* '}'
    auto r = theme_rule_set{};

    if (auto selector = parse_theme_selector(it, last, context)) {
        r.selector = *selector;
    } else {
        return std::nullopt;
    }

    if (it != last and *it == '{') {
        ++it;
    } else {
        throw parse_error(std::format("{} Missing '{{' while parsing rule-set.", token_location(it, last, context.path)));
    }

    while (it != last and *it != '}') {
        if (auto macro_declaration = parse_theme_macro_expansion(it, last, context)) {
            r.declarations.insert(r.declarations.end(), macro_declaration->begin(), macro_declaration->end());

        } else if (auto rule_declaration = parse_theme_declaration(it, last, context)) {
            // A single declaration such as "margin" will generate multiple declarations:
            // "margin-left", "margin-right", "margin-top", "margin-bottom".
            r.declarations.insert(r.declarations.end(), rule_declaration->begin(), rule_declaration->end());
        } else {
            throw parse_error(
                std::format("{} Missing declaration while parsing rule-set.", token_location(it, last, context.path)));
        }
    }

    if (it != last and *it == '}') {
        ++it;
    } else {
        throw parse_error(std::format("{} Missing '}}' while parsing rule-set.", token_location(it, last, context.path)));
    }

    return {std::move(r)};
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr bool parse_theme_color_at_rule(It& it, ItEnd last, parse_theme_context& context)
{
    if (it.size() >= 2 and it[0] == '@' and it[1] == token::id and it[1] == "color") {
        it += 2;

        if (it == last and *it != token::id) {
            throw parse_error(std::format("{} Expect name while parsing @color.", token_location(it, last, context.path)));
        }

        hilet name = static_cast<std::string>(*it);
        ++it;

        if (color::find(name) == nullptr) {
            throw parse_error(std::format(
                "{} Undefined color-name \"{}\" while parsing @color declaration.",
                token_location(it, last, context.path),
                name));
        }

        if (it == last or *it != ':') {
            throw parse_error(std::format(
                "{} Missing ':' after color-name of @color {} declaration.", token_location(it, last, context.path), name));
        }
        ++it;

        if (auto color = parse_theme_color(it, last, context)) {
            if (not context.set_color(name, *color)) {
                throw parse_error(
                    std::format("{} @color {} was already declared earlier.", token_location(it, last, context.path), name));
            }
        } else {
            throw parse_error(
                std::format("{} Missing color-value in @color {} declaration.", token_location(it, last, context.path), name));
        }

        if (it == last or *it != ';') {
            throw parse_error(
                std::format("{} Missing ';' after @color {} declaration.", token_location(it, last, context.path), name));
        }
        ++it;
        return true;

    } else {
        return false;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr bool parse_theme_let_at_rule(It& it, ItEnd last, parse_theme_context& context)
{
    // let := '@' "let" let-name ':' value ';'
    if (it.size() < 2 or it[0] != '@' or it[1] != token::id or it[1] != "let") {
        return false;
    }
    it += 2;

    if (it == last or *it != token::id) {
        throw parse_error(std::format("{} Expect a name after @let.", token_location(it, last, context.path)));
    }
    hilet let_name = static_cast<std::string>(*it);
    ++it;

    if (it == last or *it != ':') {
        throw parse_error(std::format("{} Expect ':' after @let {}.", token_location(it, last, context.path), let_name));
    }
    ++it;

    if (auto value = parse_theme_value(it, last, context)) {
        if (not context.set_let(let_name, *value)) {
            throw parse_error(
                std::format("{} @let {} was already declared earlier.", token_location(it, last, context.path), let_name));
        }
    } else {
        throw parse_error(std::format("{} Expect value after @let {} :.", token_location(it, last, context.path), let_name));
    }

    if (it == last or *it != ';') {
        throw parse_error(
            std::format("{} Expect ';' after @let {} declaration.", token_location(it, last, context.path), let_name));
    }
    ++it;
    return true;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr bool parse_theme_macro_at_rule(It& it, ItEnd last, parse_theme_context& context)
{
    // macro := '@' "macro" macro-name '{' declaration* '}'

    if (it.size() < 2 or it[0] != '@' or it[1] != token::id or it[1] != "macro") {
        return false;
    }

    it += 2;

    if (it == last or *it != token::id) {
        throw parse_error(std::format("{} Expect a name after @macro.", token_location(it, last, context.path)));
    }
    hilet macro_name = static_cast<std::string>(*it);
    ++it;

    if (it == last or *it != '{') {
        throw parse_error(std::format("{} Expect '{{' after a @macro {}.", token_location(it, last, context.path), macro_name));
    }
    ++it;

    auto declarations = std::vector<theme_declaration>{};
    while (it != last and *it != '}') {
        if (auto macro_declaration = parse_theme_macro_expansion(it, last, context)) {
            declarations.insert(declarations.end(), macro_declaration->begin(), macro_declaration->end());

        } else if (auto rule_declaration = parse_theme_declaration(it, last, context)) {
            // A single declaration such as "margin" will generate multiple declarations:
            // "margin-left", "margin-right", "margin-top", "margin-bottom".
            declarations.insert(declarations.end(), rule_declaration->begin(), rule_declaration->end());
        } else {
            throw parse_error(std::format(
                "{} Missing declaration while parsing @macro {}.", token_location(it, last, context.path), macro_name));
        }
    }

    if (it == last or *it != '}') {
        throw parse_error(
            std::format("{} Expect '}}' after a @macro {} declarations.", token_location(it, last, context.path), macro_name));
    }
    ++it;

    if (not context.set_macro(macro_name, std::move(declarations))) {
        throw parse_error(
            std::format("{} @macro {} was already declared earlier.", token_location(it, last, context.path), macro_name));
    }
    return true;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr bool parse_theme_at_rule(It& it, ItEnd last, parse_theme_context& context)
{
    if (parse_theme_color_at_rule(it, last, context)) {
        return true;
    } else if (parse_theme_let_at_rule(it, last, context)) {
        return true;
    } else if (parse_theme_macro_at_rule(it, last, context)) {
        return true;
    } else {
        return false;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_style_sheet> parse_theme_stylesheet(It& it, ItEnd last, parse_theme_context& context)
{
    // stylesheet := ( at_rule | rule_set )*
    auto r = theme_style_sheet{};

    while (it != last) {
        if (parse_theme_at_rule(it, last, context)) {
            // At rules only update the context.
        } else if (auto rule_set = parse_theme_rule_set(it, last, context)) {
            r.rule_sets.push_back(*rule_set);
        }
    }

    return r;
}

} // namespace detail

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr theme_style_sheet parse_theme(It first, ItEnd last, std::filesystem::path const& path)
{
    auto lexer_it = lexer<lexer_config::css_style()>.parse(first, last);
    auto lookahead_it = make_lookahead_iterator<4>(lexer_it, std::default_sentinel);

    auto context = detail::parse_theme_context{};
    context.path = path;
    if (auto stylesheet = detail::parse_theme_stylesheet(lookahead_it, std::default_sentinel, context)) {
        stylesheet->colors = context.move_colors();
        return *stylesheet;
    } else {
        throw parse_error(
            std::format("{} Could not parse theme file.", token_location(lookahead_it, std::default_sentinel, path)));
    }
}

[[nodiscard]] constexpr theme_style_sheet parse_theme(std::string_view str, std::filesystem::path const& path)
{
    return parse_theme(str.begin(), str.end(), path);
}

[[nodiscard]] inline theme_style_sheet parse_theme(std::filesystem::path const& path)
{
    auto view = file_view{path};
    return parse_theme(as_string_view(view), path);
}
}} // namespace hi::v1
