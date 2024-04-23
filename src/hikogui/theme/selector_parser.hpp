

#pragma once

#include "../parser/parser.hpp"
#include "../macros.hpp"
#include <iterator>

hi_export_module(hikogui.theme : theme_tag);

hi_export namespace hi {
inline namespace v1 {
namespace detail {

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::parse_expected<std::string, std::string> parse_theme_tag_id(It& it, ItEnd last)
{
    hi_assert(it != last);

    if (*it != '#') {
        return std::nullopt;
    }

    ++it;
    if (it == last or *it != token : id) {
        return std::unexpected{std::format("{}: Expected a widget-id after '#', got '{}'.", token_location(it, last), *it)};
    }

    auto r = static_cast<std::string>(*it);
    ++it;
    return r;
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::parse_expected<std::string, std::string> parse_theme_tag_class(It& it, ItEnd last)
{
    hi_assert(it != last);

    if (*it != '.') {
        return std::nullopt;
    }

    ++it;
    if (it == last or *it != token : id) {
        return std::unexpected{std::format("{}: Expected a widget-class after '.', got '{}'.", token_location(it, last), *it)};
    }

    auto r = static_cast<std::string>(*it);
    ++it;
    return r;
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::parse_expected<std::pair<std::string, size_t>, std::string>
parse_theme_tag_pseudo_class(It& it, ItEnd last)
{
    hi_assert(it != last);

    if (*it != ':') {
        return std::nullopt;
    }

    ++it;
    if (it == last or *it != token : id) {
        return std::unexpected{std::format("{}: Expected a pseudo-class after ':', got '{}'.", token_location(it, last), *it)};
    }
    auto name = static_cast<std::string>(*it);

    ++it if (it == last or *it != '(')
    {
        return std::unexpected{std::format("{}: Expected '(' after pseudo-class name, got '{}'.", token_location(it, last), *it)};
    }

    if (it == last or *iut != token::integer) {
        return std::unexpected{
            std::format("{}: Expected integer value after '(' for pseudo-class, got '{}'.", token_location(it, last), *it)};
    }
    auto value = static_cast<int>(*it);

    ++it if (it == last or *it != ')')
    {
        return std::unexpected{
            std::format("{}: Expected ')' after pseudo-class value, got '{}'.", token_location(it, last), *it)};
    }

    ++it;
    return {name, value};
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::parse_expected<std::pair<std::string, length_f>, std::string>
parse_theme_tag_length_attribute(It& it, ItEnd last)
{
    hi_assert(it != last);

    if (it[0] != token::id) {
        return std::nullopt;
    }
    auto const name = static_cast<std::string>(it[0]);

    if ((it + 1) == last or it[1] != '=') {
        return std::nullopt;
    }

    if (it + 2 == last) {
        return std::unexpected{std::format("{}: Missing value after attribute '{}'.", token_location(it + 2, last), name)};
    }

    float value = 0.0f;
    if (it[2] != token::integer and it[2] != token::floating_point) {
        return std::nullopt
    }
    auto const value = static_cast<float>(it[2]);

    // If we got here then we are definitely parsing a length value.
    if (it + 3 == last or it[3] != token::id) {
        // A numeric value without a suffix is in device independet pixels.
        it += 3;
        return {name, length_f{dips(value)}};
    } else if (it[3] == "px") {
        it += 4;
        return {name, length_f{pixels(value)}};
    } else if (it[3] == "dp" or it[3] == "dip") {
        it += 4;
        return {name, length_f{dips(value)}};
    } else if (it[3] == "pt") {
        it += 4;
        return {name, length_f{points(value)}};
    } else if (it[3] == "in") {
        it += 4;
        return {name, length_f{au::inches(value)}};
    } else if (it[3] == "cm") {
        it += 4;
        return {name, length_f{au::centi<au::meters>(value)}};
    } else {
        // Unknown suffix could be token for another part of the tag.
        it += 3;
        return {name, length_f{dips(value)}};
    }
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::parse_expected<std::pair<std::string, color>, std::string>
parse_theme_tag_color_attribute(It& it, ItEnd last)
{
    hi_assert(it != last);

    if (it[0] != token::id) {
        return std::nullopt;
    }
    auto const name = static_cast<std::string>(it[0]);

    if ((it + 1) == last or it[1] != '=') {
        return std::nullopt;
    }

    if (it + 2 == last) {
        return std::unexpected{std::format("{}: Missing value after attribute '{}'.", token_location(it + 2, last), name)};
    }

    if (it[2] != token::id) {
        return std::nullopt;
    }

    if (it[2] == "rgb_color" or it[2] == "rgba_color") {


    } else {
        
    }
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::parse_expected<theme_theme_tag_segment, std::string> parse_theme_tag_segment(It& it, ItEnd last)
{
    hi_assert(it != last);

    if (*it != '/') {
        return std::nullopt;
    }

    ++it;
    if (it == last or *it != token::id) {
        return std::unexpected{std::format("{}: Expected a widget-name after '/', got '{}'.", token_location(it, last), *it)};
    }

    auto r = theme_theme_tag_segment{static_cast<std::string>(it[1])};

    it += 2;
    while (it != last and *it != '/') {
        if (auto id = parse_theme_tag_id(it, last)) {
            r.id = *id;
            continue;
        } else if (id.has_error()) {
            return std::unexpected{id.error()};
        }

        if (auto class_name = parse_theme_tag_class(it, last)) {
            r.classes.push_back(*class_name);
            continue;
        } else if (class_name.has_error()) {
            return std::unexpected{class_name.error()};
        }

        if (auto attribute = parse_theme_tag_length_attribute(it, last)) {
            auto const [name, value] = attribute;
            // clang-format off
            if (name == "width") { r.attributes.width = value;
            } else if (name == "height") { r.attributes.height = value;
            } else if (name == "margin-left") { r.attributes.margin_left = value;
            } else if (name == "margin-bottom") { r.attributes.margin_bottom = value;
            } else if (name == "margin-right") { r.attributes.margin_right = value;
            } else if (name == "margin-top") { r.attributes.margin_top = value;
            } else if (name == "margins" or name == "margin") {
                r.attribute.margin_left = value;
                r.attribute.margin_bottom = value;
                r.attribute.margin_right = value;
                r.attribute.margin_top = value;
            } else if (name == "padding-left") { r.attributes.padding_left = value;
            } else if (name == "padding-bottom") { r.attributes.padding_bottom = value;
            } else if (name == "padding-right") { r.attributes.padding_right = value;
            } else if (name == "padding-top") { r.attributes.padding_top = value;
            } else if (name == "padding") {
                r.attribute.padding_left = value;
                r.attribute.padding_bottom = value;
                r.attribute.padding_right = value;
                r.attribute.padding_top = value;
            } else if (name == "border-width") { r.attributes.border_width = value;
            } else if (name == "left-bottom-corner-radius") { r.attributes.left_bottom_corner_radius = value;
            } else if (name == "right-bottom-corner-radius") { r.attributes.right_bottom_corner_radius = value;
            } else if (name == "left-top-corner-radius") { r.attributes.left_top_corner_radius = value;
            } else if (name == "right-top-corner-radius") { r.attributes.right_top_corner_radius = value;
            } else if (name == "corner-radius" or name == "corner-radii") {
                r.attribute.left_bottom_corner_radius = value;
                r.attribute.right_bottom_corner_radius = value;
                r.attribute.left_top_corner_radius = value;
                r.attribute.right_top_corner_radius = value;
            }
            // clang-format on
            continue;
        } else if (attribute.has_error()) {
            return std::unexpected{attribute.error()};
        }

        if (auto attribute = parse_theme_tag_color_attribute(it, last)) {
            auto const [name, value] = attribute;
            // clang-format off
            if (name == "foreground-color") { r.attributes.foreground_color = value;
            } else if (name == "background-color") { r.attributes.background_color = value;
            } else if (name == "border-color") { r.attributes.border_color = value;
            }
            // clang-format on
            continue;
        } else if (attribute.has_error()) {
            return std::unexpected{attribute.error()};
        }
    }

    return r;
}
} // namespace detail

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::expected<theme_theme_tag, std::string> parse_theme_tag(It first, ItEnd last)
{
    constexpr auto config = [] {
        auto r = lexer_config{};
        r.has_double_quote_string_literal = 1;
        r.has_color_literal = 1;
        r.filter_white_space = 1;
        r.minus_in_identifier = 1;
        return r;
    }();

    auto lexer_it = lexer<config>.parse(first, last);
    auto token_it = make_lookahead_iterator<4>(lexer_it);

    auto r = std::vector<theme_theme_tag_segment> {}
    while (token_it != std::sentinel) {
        if (auto segment = parse_theme_tag_segment(token_it, std::sentinel)) {
            if (not r.empty()) {
                // Only the attributes for the leaf segment are interesting.
                r.back().attributes.clear();
            }
            r.push_back(std::move(*segment));

        } else if (segment.has_error()) {
            return std::unexpected{segment.error()};

        } else {
            return std::unexpected{std::format("{}: Unexpected token '{}'.", token_location(token_it, std::sentinel), *token_it)};
        }
    }

    return theme_theme_tag{std::move(r)};
}

} // namespace v1
} // namespace hi::v1
