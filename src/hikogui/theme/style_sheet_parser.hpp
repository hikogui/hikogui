// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../parser/module.hpp"
#include "theme_mode.hpp"
#include "theme_length.hpp"
#include "style_sheet.hpp"
#include <string>
#include <vector>

namespace hi { inline namespace v1 {
namespace detail {

class style_sheet_parser_context {
public:
    std::filesystem::path path;

    [[nodiscard]] constexpr bool set_macro(std::string const& name, std::vector<style_sheet_declaration> declarations) noexcept
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

    constexpr std::optional<std::vector<style_sheet_declaration>> get_macro(std::string const& name) const noexcept
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

    [[nodiscard]] constexpr bool set_let(std::string const& name, style_sheet_value value) noexcept
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

    constexpr std::optional<style_sheet_value> get_let(std::string const& name) const noexcept
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
    std::vector<std::pair<std::string, std::vector<style_sheet_declaration>>> _macros;
    std::vector<std::pair<std::string, style_sheet_value>> _lets;
};

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<hi::language_tag>
parse_style_sheet_theme_state_lang(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (*it == token::id and *it == "lang") {
        ++it;

        if (it == last or *it != '(') {
            throw parse_error(std::format("{} Missing '(' after ':lang'.", token_location(it, last, context.path)));
        }
        ++it;

        auto language_tag_string = std::string{};
        while (it != last and *it != ')') {
            if (*it == '*') {
                language_tag_string += '*';
                ++it;
            } else if (*it == '-') {
                language_tag_string += '-';
                ++it;
            } else if (*it == token::id) {
                language_tag_string += static_cast<std::string>(*it);
                ++it;
            } else {
                throw parse_error(std::format(
                    "{} Unexpected token while parsing argument of ':lang()'.", token_location(it, last, context.path)));
            }
        }

        auto r = [&] {
            try {
                return language_tag{language_tag_string};
            } catch (std::exception const& e) {
                throw parse_error(std::format(
                    "{} Invalid language-tag '{}' while parsing argument of ':lang()'. {}",
                    token_location(it, last, context.path),
                    language_tag_string, e.what()));
            }
        }();

        if (it == last or *it != ')') {
            throw parse_error(std::format("{} Missing ')' at end of ':lang'.", token_location(it, last, context.path)));
        }
        ++it;

        return r;
    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<hi::text_phrasing_mask>
parse_style_sheet_theme_state_phrasing(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (*it == token::id and *it == "phrasing") {
        ++it;

        if (it == last or *it != '(') {
            throw parse_error(std::format("{} Missing '(' after ':phrasing'.", token_location(it, last, context.path)));
        }
        ++it;

        if (it == last or *it != token::id) {
            throw parse_error(std::format("{} Missing integer after ':phrasing('.", token_location(it, last, context.path)));
        }

        hilet phrasing_mask = [&] {
            try {
                return to_text_phrasing_mask(static_cast<std::string>(*it));
            } catch (std::exception const& e) {
                throw parse_error(std::format(
                    "{} Could not convert argument '{}' of ':phrasing()' to integer. {}",
                    token_location(it, last, context.path),
                    static_cast<std::string>(*it),
                    e.what()));
            }
        }();
        ++it;

        if (it == last or *it != ')') {
            throw parse_error(std::format("{} Missing ')' at end of ':phrasing'.", token_location(it, last, context.path)));
        }
        ++it;

        return phrasing_mask;
    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<std::pair<theme_state, theme_state_mask>>
parse_style_sheet_theme_state(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (*it == token::id and *it == "disabled") {
        ++it;
        return std::pair{theme_state::disabled, theme_state_mask::mouse};
    } else if (*it == token::id and *it == "enabled") {
        ++it;
        return std::pair{theme_state::enabled, theme_state_mask::mouse};
    } else if (*it == token::id and *it == "hover") {
        ++it;
        return std::pair{theme_state::hover, theme_state_mask::mouse};
    } else if (*it == token::id and *it == "active") {
        ++it;
        return std::pair{theme_state::active, theme_state_mask::mouse};
    } else if (*it == token::id and *it == "no-focus") {
        ++it;
        return std::pair{theme_state::no_focus, theme_state_mask::focus};
    } else if (*it == token::id and *it == "focus") {
        ++it;
        return std::pair{theme_state::focus, theme_state_mask::focus};
    } else if (*it == token::id and *it == "off") {
        ++it;
        return std::pair{theme_state::off, theme_state_mask::value};
    } else if (*it == token::id and *it == "on") {
        ++it;
        return std::pair{theme_state::on, theme_state_mask::value};
    } else if (*it == token::id and *it == "layer") {
        ++it;

        if (it == last or *it != '(') {
            throw parse_error(std::format("{} Missing '(' after ':layer'.", token_location(it, last, context.path)));
        }
        ++it;

        if (it == last or *it != token::integer) {
            throw parse_error(std::format("{} Missing integer after ':layer('.", token_location(it, last, context.path)));
        }

        hilet layer_nr = [&] {
            try {
                return static_cast<uint8_t>(*it);
            } catch (std::exception const& e) {
                throw parse_error(std::format(
                    "{} Could not convert argument of ':layer()' to integer. {}",
                    token_location(it, last, context.path),
                    e.what()));
            }
        }();
        ++it;

        hilet layer_state = [&] {
            switch (layer_nr) {
            case 0:
                return theme_state::layer_0;
            case 1:
                return theme_state::layer_1;
            case 2:
                return theme_state::layer_2;
            case 3:
                return theme_state::layer_3;
            default:
                throw parse_error(std::format(
                    "{} Expect ':layer()' value of 0, 1, 2 or 3, got {}.", token_location(it, last, context.path), layer_nr));
            }
        }();

        if (it == last or *it != ')') {
            throw parse_error(std::format("{} Missing ')' at end of ':layer'.", token_location(it, last, context.path)));
        }
        ++it;

        return std::pair{layer_state, theme_state_mask::layers};

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<style_sheet_pattern>
parse_style_sheet_pattern(It& it, ItEnd last, style_sheet_parser_context& context)
{
    // pattern := ( id | '*' ) ( '>'? ( id | '*' ) )*  ( ':' id )*
    auto r = style_sheet_pattern{};

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

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<style_sheet_selector>
parse_style_sheet_selector(It& it, ItEnd last, style_sheet_parser_context& context)
{
    // selector := pattern (',' pattern)*
    auto r = style_sheet_selector{};

    if (auto pattern = parse_style_sheet_pattern(it, last, context)) {
        r.push_back(*pattern);
    } else {
        return std::nullopt;
    }

    while (it != last and *it == ',') {
        ++it;

        if (auto pattern = parse_style_sheet_pattern(it, last, context)) {
            r.push_back(*pattern);
        } else {
            throw parse_error(std::format("{} Missing pattern after ',' in selector.", token_location(it, last, context.path)));
        }
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<float>
parse_style_sheet_color_component(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto r = 0.0f;

    if (it.size() >= 2 and (it[0] == token::integer or it[0] == token::real) and it[1] == '%') {
        r = sRGB_gamma_to_linear(static_cast<float>(it[0]) * 0.01f);
        it += 2;
    } else if (it != last and *it == token::real) {
        r = static_cast<float>(*it);
        ++it;
    } else if (it.size() >= 2 and it[0] == '-' and it[1] == token::real) {
        r = -static_cast<float>(it[1]);
        it += 2;
    } else if (it != last and *it == token::integer) {
        r = sRGB_gamma_to_linear(static_cast<float>(*it) / 255.0f);
        ++it;
    } else {
        return std::nullopt;
    }

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<float>
parse_style_sheet_alpha_component(It& it, ItEnd last, style_sheet_parser_context& context)
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
[[nodiscard]] constexpr std::optional<color> parse_style_sheet_color(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (it != last and *it == token::color) {
        try {
            auto r = static_cast<color>(*it);
            ++it;
            return r;

        } catch (std::exception const& e) {
            throw parse_error(std::format(
                "{} Invalid color literal '{}': {}",
                token_location(it, last, context.path),
                static_cast<std::string>(*it),
                e.what()));
        }

    } else if (it != last and *it == token::id and *it == "rgb") {
        // rgb-color := "rgb" '(' color-component ','? color-component ','? color-component ( [,/]? alpha-component )? ')'
        // color-component := integer | float | number '%'
        // alpha-component := float | number '%'
        ++it;

        if (it == last or *it != '(') {
            throw parse_error(
                std::format("{} Expect '(' after \"color-layers\" keyword.", token_location(it, last, context.path)));
        }
        ++it;

        auto r = color{};
        r.a() = 1.0;

        if (auto component = parse_style_sheet_color_component(it, last, context)) {
            r.r() = *component;
        } else {
            throw parse_error(std::format("{} Expect a red-color-component after '('.", token_location(it, last, context.path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }

        if (auto component = parse_style_sheet_color_component(it, last, context)) {
            r.g() = *component;
        } else {
            throw parse_error(std::format(
                "{} Expect a green-color-component after red-color-component.", token_location(it, last, context.path)));
        }

        if (it != last and *it == ',') {
            ++it;
        }

        if (auto component = parse_style_sheet_color_component(it, last, context)) {
            r.b() = *component;
        } else {
            throw parse_error(std::format(
                "{} Expect a blue-color-component after red-color-component.", token_location(it, last, context.path)));
        }

        if (it != last and (*it == ',' or *it == '/')) {
            ++it;
        }

        // Alpha is optional.
        if (auto component = parse_style_sheet_alpha_component(it, last, context)) {
            r.a() = *component;
        }

        if (it == last or *it != ')') {
            throw parse_error(std::format("{} Expect ')' after colors-components.", token_location(it, last, context.path)));
        }
        ++it;
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
[[nodiscard]] constexpr std::vector<color> parse_style_sheet_colors(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto r = std::vector<color>{};

    if (auto color = parse_style_sheet_color(it, last, context)) {
        r.push_back(*color);
    } else {
        return r;
    }

    if (it != last and *it == ',') {
        ++it;
    }

    while (it != last and *it != ';' and *it != '!') {
        if (auto color = parse_style_sheet_color(it, last, context)) {
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
[[nodiscard]] constexpr std::optional<theme_length>
parse_style_sheet_length(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (it.size() >= 2 and (it[0] == token::integer or it[0] == token::real) and it[1] == token::id) {
        hilet r = [&]() -> theme_length {
            if (it[1] == "dp") {
                return dips{static_cast<double>(it[0])};
            } else if (it[1] == "pt") {
                return dips{points{static_cast<double>(it[0])}};
            } else if (it[1] == "mm") {
                return dips{millimeters{static_cast<double>(it[0])}};
            } else if (it[1] == "cm") {
                return dips{centimeters{static_cast<double>(it[0])}};
            } else if (it[1] == "dm") {
                return dips{decimeters{static_cast<double>(it[0])}};
            } else if (it[1] == "m") {
                return dips{meters{static_cast<double>(it[0])}};
            } else if (it[1] == "in") {
                return dips{inches{static_cast<double>(it[0])}};
            } else if (it[1] == "px") {
                return pixels(static_cast<double>(it[0]));
            } else if (it[1] == "em") {
                return em_quads(static_cast<double>(it[0]));
            } else {
                throw parse_error(std::format(
                    "{} Expected either \"dp\", \"pt\", \"mm\", \"cm\", \"dm\", \"m\", \"in\", \"px\" or \"em\" after number",
                    token_location(it, last, context.path)));
            }
        }();

        it += 2;
        return r;

    } else if (it != last and (*it == token::integer or *it == token::real)) {
        // Implicitly a number without suffix is in `dp`.
        hilet r = theme_length{dips{static_cast<float>(*it)}};
        ++it;
        return r;

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<theme_length>
parse_style_sheet_lengths(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto r = std::vector<theme_length>{};

    if (auto length = parse_style_sheet_length(it, last, context)) {
        r.push_back(*length);
    } else {
        return r;
    }

    if (it != last and *it == ',') {
        ++it;
    }

    while (it != last and *it != ';' and *it != '!') {
        if (auto length = parse_style_sheet_length(it, last, context)) {
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
[[nodiscard]] constexpr std::optional<style_sheet_value>
parse_style_sheet_let_expansion(It& it, ItEnd last, style_sheet_parser_context& context)
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
[[nodiscard]] constexpr std::optional<style_sheet_value>
parse_style_sheet_value(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (auto value = parse_style_sheet_let_expansion(it, last, context)) {
        return value;

    } else if (auto color = parse_style_sheet_color(it, last, context)) {
        return style_sheet_value{*color};

    } else if (auto length = parse_style_sheet_length(it, last, context)) {
        return style_sheet_value{*length};

    } else {
        return std::nullopt;
    }
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<style_sheet_declaration>
parse_style_sheet_font_family_declaration(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto family_id = font_family_id{};
    while (it != last and *it != ';' and *it != '!') {
        if (*it == ',') {
            // Ignore optional ','
            ++it;

        } else if (family_id) {
            // If family_id was already found, just consume tokens until ';' or '!' is found.
            ++it;

        } else if (*it == token::id and *it == "serif") {
            if (family_id = find_font_family("Times New Roman")) {
            } else if (family_id = find_font_family("Big Caslon")) {
            } else if (family_id = find_font_family("Bodoni MT")) {
            } else if (family_id = find_font_family("Book Antique")) {
            } else if (family_id = find_font_family("Bookman")) {
            } else if (family_id = find_font_family("New Century Schoolbook")) {
            } else if (family_id = find_font_family("Calisto MT")) {
            } else if (family_id = find_font_family("Cambria")) {
            } else if (family_id = find_font_family("Didot")) {
            } else if (family_id = find_font_family("Garamond")) {
            } else if (family_id = find_font_family("Georgia")) {
            } else if (family_id = find_font_family("Goudy Old Style")) {
            } else if (family_id = find_font_family("Hoeflet Text")) {
            } else if (family_id = find_font_family("Lucida Bright")) {
            } else if (family_id = find_font_family("Palatino")) {
            } else if (family_id = find_font_family("Perpetua")) {
            } else if (family_id = find_font_family("Rockwell")) {
            } else if (family_id = find_font_family("Baskerville")) {
            } else {
                throw parse_error(
                    std::format("{} Could not find any serif fallback font.", token_location(it, last, context.path)));
            }
            ++it;

        } else if (*it == token::id and *it == "sans-serif") {
            if (family_id = find_font_family("Arial")) {
            } else if (family_id = find_font_family("Helvetica")) {
            } else if (family_id = find_font_family("Verdana")) {
            } else if (family_id = find_font_family("Calibri")) {
            } else if (family_id = find_font_family("Noto")) {
            } else if (family_id = find_font_family("Lucida Sans")) {
            } else if (family_id = find_font_family("Gill Sans")) {
            } else if (family_id = find_font_family("Century Gothic")) {
            } else if (family_id = find_font_family("Candara")) {
            } else if (family_id = find_font_family("Futara")) {
            } else if (family_id = find_font_family("Franklin Gothic Medium")) {
            } else if (family_id = find_font_family("Trebuchet MS")) {
            } else if (family_id = find_font_family("Geneva")) {
            } else if (family_id = find_font_family("Segoe UI")) {
            } else if (family_id = find_font_family("Optima")) {
            } else if (family_id = find_font_family("Avanta Garde")) {
            } else {
                throw parse_error(
                    std::format("{} Could not find any sans-serif fallback font.", token_location(it, last, context.path)));
            }
            ++it;

        } else if (*it == token::id and *it == "monospace") {
            if (family_id = find_font_family("Consolas")) {
            } else if (family_id = find_font_family("Courier")) {
            } else if (family_id = find_font_family("Courier New")) {
            } else if (family_id = find_font_family("Lucida Console")) {
            } else if (family_id = find_font_family("Lucidatypewriter")) {
            } else if (family_id = find_font_family("Lucida Sans Typewriter")) {
            } else if (family_id = find_font_family("Monaco")) {
            } else if (family_id = find_font_family("Andale Mono")) {
            } else {
                throw parse_error(
                    std::format("{} Could not find any monospace fallback font.", token_location(it, last, context.path)));
            }
            ++it;

        } else if (*it == token::id and *it == "cursive") {
            if (family_id = find_font_family("Comic Sans")) {
            } else if (family_id = find_font_family("Comic Sans MS")) {
            } else if (family_id = find_font_family("Apple Chancery")) {
            } else if (family_id = find_font_family("Zapf Chancery")) {
            } else if (family_id = find_font_family("Bradly Hand")) {
            } else if (family_id = find_font_family("Brush Script MT")) {
            } else if (family_id = find_font_family("Brush Script Std")) {
            } else if (family_id = find_font_family("Snell Roundhan")) {
            } else if (family_id = find_font_family("URW Chancery")) {
            } else if (family_id = find_font_family("Coronet script")) {
            } else if (family_id = find_font_family("Florence")) {
            } else if (family_id = find_font_family("Parkavenue")) {
            } else {
                throw parse_error(
                    std::format("{} Could not find any monospace fallback font.", token_location(it, last, context.path)));
            }
            ++it;

        } else if (*it == token::id and *it == "fantasy") {
            if (family_id = find_font_family("Impact")) {
            } else if (family_id = find_font_family("Brushstroke")) {
            } else if (family_id = find_font_family("Luminari")) {
            } else if (family_id = find_font_family("Chalkduster")) {
            } else if (family_id = find_font_family("Jazz LET")) {
            } else if (family_id = find_font_family("Blippo")) {
            } else if (family_id = find_font_family("Stencil Std")) {
            } else if (family_id = find_font_family("Market Felt")) {
            } else if (family_id = find_font_family("Trattatello")) {
            } else if (family_id = find_font_family("Arnoldboecklin")) {
            } else if (family_id = find_font_family("Oldtown")) {
            } else if (family_id = find_font_family("Copperplate")) {
            } else if (family_id = find_font_family("papyrus")) {
            } else {
                throw parse_error(
                    std::format("{} Could not find any fantasy fallback font.", token_location(it, last, context.path)));
            }
            ++it;

        } else if (*it == token::dstr) {
            family_id = find_font_family(static_cast<std::string>(*it));
            ++it;

        } else {
            throw parse_error(std::format(
                "{} Expecting a font-family name or serif, sans-serif, monospace, cursive or fantasy.",
                token_location(it, last, context.path)));
        }
    }

    if (not family_id) {
        throw parse_error(std::format(
            "{} Could not find any of the fonts in this font-family declaration.", token_location(it, last, context.path)));
    }

    auto r = std::vector<style_sheet_declaration>{};
    r.emplace_back(style_sheet_declaration_name::font_family, family_id);
    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<style_sheet_declaration>
parse_style_sheet_font_style_declaration(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto r = std::vector<style_sheet_declaration>{};

    if (it != last and *it == token::id and *it == "normal") {
        r.emplace_back(style_sheet_declaration_name::font_style, font_style::normal);
    } else if (it != last and *it == token::id and *it == "italic") {
        r.emplace_back(style_sheet_declaration_name::font_style, font_style::italic);
    } else if (it != last and *it == token::id and *it == "oblique") {
        r.emplace_back(style_sheet_declaration_name::font_style, font_style::oblique);
    } else {
        throw parse_error(std::format(
            "{} Expecting normal, italic or oblique as value of a font-style declaration.",
            token_location(it, last, context.path)));
    }

    ++it;
    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<style_sheet_declaration>
parse_style_sheet_font_weight_declaration(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto r = std::vector<style_sheet_declaration>{};

    if (it != last and *it == token::id and *it == "thin") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::thin);
    } else if (it != last and *it == token::id and *it == "extra-light") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::extra_light);
    } else if (it != last and *it == token::id and *it == "light") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::light);
    } else if (it != last and *it == token::id and (*it == "regular" or *it == "normal")) {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::regular);
    } else if (it != last and *it == token::id and *it == "medium") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::medium);
    } else if (it != last and *it == token::id and *it == "semi-bold") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::semi_bold);
    } else if (it != last and *it == token::id and *it == "bold") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::bold);
    } else if (it != last and *it == token::id and *it == "extra-bold") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::extra_bold);
    } else if (it != last and *it == token::id and *it == "black") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::black);
    } else if (it != last and *it == token::id and *it == "extra-black") {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight::extra_black);

    } else if (it != last and *it == token::integer) {
        r.emplace_back(style_sheet_declaration_name::font_weight, font_weight_from_int(static_cast<int>(*it)));

    } else {
        throw parse_error(std::format(
            "{} Expecting a integer or  value of a font-weight declaration.", token_location(it, last, context.path)));
    }

    ++it;
    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::vector<style_sheet_declaration>
parse_style_sheet_margin_declarations(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto r = std::vector<style_sheet_declaration>{};

    if (auto lengths = parse_style_sheet_lengths(it, last, context); not lengths.empty()) {
        if (lengths.size() == 1) {
            r.emplace_back(style_sheet_declaration_name::margin_top, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::margin_right, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::margin_bottom, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::margin_left, lengths[0]);
        } else if (lengths.size() == 2) {
            r.emplace_back(style_sheet_declaration_name::margin_top, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::margin_right, lengths[1]);
            r.emplace_back(style_sheet_declaration_name::margin_bottom, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::margin_left, lengths[1]);
        } else if (lengths.size() == 3) {
            r.emplace_back(style_sheet_declaration_name::margin_top, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::margin_right, lengths[1]);
            r.emplace_back(style_sheet_declaration_name::margin_bottom, lengths[2]);
            r.emplace_back(style_sheet_declaration_name::margin_left, lengths[1]);
        } else if (lengths.size() == 4) {
            r.emplace_back(style_sheet_declaration_name::margin_top, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::margin_right, lengths[1]);
            r.emplace_back(style_sheet_declaration_name::margin_bottom, lengths[2]);
            r.emplace_back(style_sheet_declaration_name::margin_left, lengths[3]);
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
[[nodiscard]] constexpr std::vector<style_sheet_declaration>
parse_style_sheet_border_radius_declarations(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto r = std::vector<style_sheet_declaration>{};

    if (auto lengths = parse_style_sheet_lengths(it, last, context); not lengths.empty()) {
        if (lengths.size() == 1) {
            r.emplace_back(style_sheet_declaration_name::border_top_left_radius, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::border_top_right_radius, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::border_bottom_left_radius, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::border_bottom_right_radius, lengths[0]);
        } else if (lengths.size() == 2) {
            r.emplace_back(style_sheet_declaration_name::border_top_left_radius, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::border_top_right_radius, lengths[1]);
            r.emplace_back(style_sheet_declaration_name::border_bottom_left_radius, lengths[1]);
            r.emplace_back(style_sheet_declaration_name::border_bottom_right_radius, lengths[0]);
        } else if (lengths.size() == 4) {
            r.emplace_back(style_sheet_declaration_name::border_top_left_radius, lengths[0]);
            r.emplace_back(style_sheet_declaration_name::border_top_right_radius, lengths[1]);
            r.emplace_back(style_sheet_declaration_name::border_bottom_left_radius, lengths[2]);
            r.emplace_back(style_sheet_declaration_name::border_bottom_right_radius, lengths[3]);
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
[[nodiscard]] constexpr std::vector<style_sheet_declaration>
parse_style_sheet_caret_color_declarations(It& it, ItEnd last, style_sheet_parser_context& context)
{
    auto r = std::vector<style_sheet_declaration>{};

    if (auto colors = parse_style_sheet_colors(it, last, context); not colors.empty()) {
        if (colors.size() == 1) {
            r.emplace_back(style_sheet_declaration_name::caret_primary_color, colors[0]);
            r.emplace_back(style_sheet_declaration_name::caret_secondary_color, colors[0]);
            r.emplace_back(style_sheet_declaration_name::caret_overwrite_color, colors[0]);
            r.emplace_back(style_sheet_declaration_name::caret_compose_color, colors[0]);
        } else if (colors.size() == 2) {
            r.emplace_back(style_sheet_declaration_name::caret_primary_color, colors[0]);
            r.emplace_back(style_sheet_declaration_name::caret_secondary_color, colors[1]);
            r.emplace_back(style_sheet_declaration_name::caret_overwrite_color, colors[0]);
            r.emplace_back(style_sheet_declaration_name::caret_compose_color, colors[1]);
        } else if (colors.size() == 3) {
            r.emplace_back(style_sheet_declaration_name::caret_primary_color, colors[0]);
            r.emplace_back(style_sheet_declaration_name::caret_secondary_color, colors[1]);
            r.emplace_back(style_sheet_declaration_name::caret_overwrite_color, colors[2]);
            r.emplace_back(style_sheet_declaration_name::caret_compose_color, colors[1]);
        } else if (colors.size() == 4) {
            r.emplace_back(style_sheet_declaration_name::caret_primary_color, colors[0]);
            r.emplace_back(style_sheet_declaration_name::caret_secondary_color, colors[1]);
            r.emplace_back(style_sheet_declaration_name::caret_overwrite_color, colors[2]);
            r.emplace_back(style_sheet_declaration_name::caret_compose_color, colors[3]);
        } else {
            throw parse_error(std::format(
                "{} Expect 1 to 4 color values when parsing \"caret-color\" declaration, got {}.",
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
[[nodiscard]] constexpr std::optional<std::vector<style_sheet_declaration>>
parse_style_sheet_macro_expansion(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (it.size() < 2 or it[0] != '@' or it[1] != token::id) {
        return std::nullopt;
    }

    hilet name = static_cast<std::string>(it[1]);
    it += 2;

    auto r = std::vector<style_sheet_declaration>{};
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
[[nodiscard]] constexpr std::optional<std::vector<style_sheet_declaration>>
parse_style_sheet_declaration(It& it, ItEnd last, style_sheet_parser_context& context)
{
    // declaration := id ':' value ('!' "important")? ';'
    auto r = std::vector<style_sheet_declaration>{};

    if (it.size() < 2 or it[0] != token::id or it[1] != ':') {
        return std::nullopt;
    }

    hilet name = static_cast<std::string>(*it);
    it += 2;

    if (name == "margin") {
        r = parse_style_sheet_margin_declarations(it, last, context);

    } else if (name == "border-radius") {
        r = parse_style_sheet_border_radius_declarations(it, last, context);

    } else if (name == "caret-color") {
        r = parse_style_sheet_caret_color_declarations(it, last, context);

    } else if (name == "font-family") {
        r = parse_style_sheet_font_family_declaration(it, last, context);

    } else if (name == "font-style") {
        r = parse_style_sheet_font_style_declaration(it, last, context);

    } else if (name == "font-weight") {
        r = parse_style_sheet_font_weight_declaration(it, last, context);

    } else {
        hilet id = [&] {
            if (auto id = style_sheet_declaration_name_metadata.at_if(name)) {
                return *id;
            } else {
                throw parse_error(std::format("{} Invalid declaration name '{}'.", token_location(it, last, context.path), name));
            }
        }();

        auto value = [&] {
            if (auto value = parse_style_sheet_value(it, last, context)) {
                return *value;

            } else {
                throw parse_error(std::format(
                    "{} Missing value after ':' while parsing {} declaration.", token_location(it, last, context.path), name));
            }
        }();

        hilet value_mask = style_sheet_declaration_name_value_mask_metadata[id];
        if (std::holds_alternative<hi::dips>(value) and not to_bool(value_mask & style_sheet_value_mask::dips)) {
            throw parse_error(std::format(
                "{} Incorrect value type 'length:pt' for declaration of '{}'", token_location(it, last, context.path), name));
        } else if (std::holds_alternative<hi::pixels>(value) and not to_bool(value_mask & style_sheet_value_mask::pixels)) {
            throw parse_error(std::format(
                "{} Incorrect value type 'length:px' for declaration of '{}'", token_location(it, last, context.path), name));
        } else if (std::holds_alternative<hi::em_quads>(value) and not to_bool(value_mask & style_sheet_value_mask::em_quads)) {
            throw parse_error(std::format(
                "{} Incorrect value type 'length:em' for declaration of '{}'", token_location(it, last, context.path), name));
        } else if (std::holds_alternative<hi::color>(value) and not to_bool(value_mask & style_sheet_value_mask::color)) {
            throw parse_error(std::format(
                "{} Incorrect value type 'color' for declaration of '{}'", token_location(it, last, context.path), name));
        } else if (
            std::holds_alternative<hi::font_family_id>(value) and
            not to_bool(value_mask & style_sheet_value_mask::font_family_id)) {
            throw parse_error(std::format(
                "{} Incorrect value type 'font family id' for declaration of '{}'",
                token_location(it, last, context.path),
                name));
        } else if (
            std::holds_alternative<hi::font_weight>(value) and not to_bool(value_mask & style_sheet_value_mask::font_weight)) {
            throw parse_error(std::format(
                "{} Incorrect value type 'font weight' for declaration of '{}'", token_location(it, last, context.path), name));
        } else if (
            std::holds_alternative<hi::font_style>(value) and not to_bool(value_mask & style_sheet_value_mask::font_style)) {
            throw parse_error(std::format(
                "{} Incorrect value type 'font style' for declaration of '{}'", token_location(it, last, context.path), name));
        }

        r.emplace_back(id, std::move(value));
    }

    // Optional !important
    if (it.size() >= 2 and it[0] == '!' and it[1] == token::id and it[1] == "important") {
        for (auto& x : r) {
            x.important = true;
        }
        it += 2;
    }

    if (it == last or *it != ';') {
        throw parse_error(std::format(
            "{} Missing ';' after value while parsing {} declaration.", token_location(it, last, context.path), name));
    }
    ++it;

    return {std::move(r)};
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<style_sheet_rule_set>
parse_style_sheet_rule_set(It& it, ItEnd last, style_sheet_parser_context& context)
{
    // rule_set := selector (':' state)* '{' declaration* '}'
    auto r = style_sheet_rule_set{};

    if (auto selector = parse_style_sheet_selector(it, last, context)) {
        r.selector = *selector;
    } else {
        return std::nullopt;
    }

    while (it != last and *it == ':') {
        ++it;

        if (auto state_and_mask = parse_style_sheet_theme_state(it, last, context)) {
            r.state |= state_and_mask->first;
            r.state_mask |= state_and_mask->second;

        } else if (auto language_tag = parse_style_sheet_theme_state_lang(it, last, context)) {
            r.language_mask = *language_tag;

        } else if (auto phrasing_mask = parse_style_sheet_theme_state_phrasing(it, last, context)) {
            r.phrasing_mask = *phrasing_mask;

        } else {
            throw parse_error(
                std::format("{} Expecting state-id after ':' in selector.", token_location(it, last, context.path)));
        }
    }

    if (it != last and *it == '{') {
        ++it;
    } else {
        throw parse_error(std::format("{} Missing '{{' while parsing rule-set.", token_location(it, last, context.path)));
    }

    while (it != last and *it != '}') {
        if (auto macro_declaration = parse_style_sheet_macro_expansion(it, last, context)) {
            r.declarations.insert(r.declarations.end(), macro_declaration->begin(), macro_declaration->end());

        } else if (auto rule_declaration = parse_style_sheet_declaration(it, last, context)) {
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
[[nodiscard]] constexpr bool parse_style_sheet_color_at_rule(It& it, ItEnd last, style_sheet_parser_context& context)
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

        if (auto color = parse_style_sheet_color(it, last, context)) {
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
[[nodiscard]] constexpr bool parse_style_sheet_let_at_rule(It& it, ItEnd last, style_sheet_parser_context& context)
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

    if (auto value = parse_style_sheet_value(it, last, context)) {
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
[[nodiscard]] constexpr bool parse_style_sheet_macro_at_rule(It& it, ItEnd last, style_sheet_parser_context& context)
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

    auto declarations = std::vector<style_sheet_declaration>{};
    while (it != last and *it != '}') {
        if (auto macro_declaration = parse_style_sheet_macro_expansion(it, last, context)) {
            declarations.insert(declarations.end(), macro_declaration->begin(), macro_declaration->end());

        } else if (auto rule_declaration = parse_style_sheet_declaration(it, last, context)) {
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
[[nodiscard]] constexpr std::optional<std::string>
parse_style_sheet_name_at_rule(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (it.size() < 2 or it[0] != '@' or it[1] != token::id or it[1] != "name") {
        return std::nullopt;
    }
    it += 2;

    if (it == last or *it != token::dstr) {
        throw parse_error(std::format("{} Expect string after @name.", token_location(it, last, context.path)));
    }
    auto r = static_cast<std::string>(*it);
    ++it;

    if (it == last or *it != ';') {
        throw parse_error(std::format("{} Expect ';' after @name \"{}\".", token_location(it, last, context.path), r));
    }
    ++it;

    return {std::move(r)};
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<theme_mode>
parse_style_sheet_mode_at_rule(It& it, ItEnd last, style_sheet_parser_context& context)
{
    if (it.size() < 2 or it[0] != '@' or it[1] != token::id or it[1] != "mode") {
        return std::nullopt;
    }
    it += 2;

    hilet r = [&] {
        if (it != last and *it == token::id and *it == "light") {
            return theme_mode::light;
        } else if (it != last and *it == token::id and *it == "dark") {
            return theme_mode::dark;
        } else {
            throw parse_error(std::format("{} Expect light or dark after @mode.", token_location(it, last, context.path)));
        }
    }();
    ++it;

    if (it == last or *it != ';') {
        throw parse_error(std::format("{} Expect ';' after @name \"{}\".", token_location(it, last, context.path), r));
    }
    ++it;

    return r;
}

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<style_sheet> parse_style_sheet(It& it, ItEnd last, style_sheet_parser_context& context)
{
    // stylesheet := ( at_rule | rule_set )*
    auto r = style_sheet{};

    if (auto name = parse_style_sheet_name_at_rule(it, last, context)) {
        r.name = std::move(*name);
    } else {
        throw parse_error(
            std::format("{} Did not find required @name as first declaration.", token_location(it, last, context.path)));
    } 

    if (auto mode = parse_style_sheet_mode_at_rule(it, last, context)) {
        r.mode = *mode;
    } else {
        throw parse_error(
            std::format("{} Did not find required @mode declaration after @name in the style sheet.", token_location(it, last, context.path)));
    }

    while (it != last) {
        if (parse_style_sheet_color_at_rule(it, last, context)) {
            // colors are directly added to the context.
        } else if (parse_style_sheet_let_at_rule(it, last, context)) {
            // lets are directly added to the context.
        } else if (parse_style_sheet_macro_at_rule(it, last, context)) {
            // macros are directly added to the context.
        } else if (auto rule_set = parse_style_sheet_rule_set(it, last, context)) {
            r.rule_sets.push_back(*rule_set);
        } else {
            throw parse_error(std::format("{} Found unexpected token.", token_location(it, last, context.path)));
        }
    }

    return r;
}

} // namespace detail

template<typename It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr style_sheet parse_style_sheet(It first, ItEnd last, std::filesystem::path const& path)
{
    auto lexer_it = lexer<lexer_config::css_style()>.parse(first, last);
    auto lookahead_it = make_lookahead_iterator<4>(lexer_it, std::default_sentinel);

    auto context = detail::style_sheet_parser_context{};
    context.path = path;
    if (auto stylesheet = detail::parse_style_sheet(lookahead_it, std::default_sentinel, context)) {
        stylesheet->colors = context.move_colors();
        return *stylesheet;
    } else {
        throw parse_error(
            std::format("{} Could not parse style sheet file.", token_location(lookahead_it, std::default_sentinel, path)));
    }
}

[[nodiscard]] constexpr style_sheet parse_style_sheet(std::string_view str, std::filesystem::path const& path)
{
    return parse_style_sheet(str.begin(), str.end(), path);
}

[[nodiscard]] inline style_sheet parse_style_sheet(std::filesystem::path const& path)
{
    return parse_style_sheet(as_string_view(file_view{path}), path);
}

}} // namespace hi::v1
