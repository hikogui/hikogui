

#pragma once

#include "style_path.hpp
#include "style_attributes.hpp"
#include "../parser/parser.hpp"
#include "../container/container.hpp"
#include "../macros.hpp"
#include <iterator>

hi_export_module(hikogui.theme : theme_tag);

hi_export namespace hi {
inline namespace v1 {
namespace detail {

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr expected_optional<std::string, std::string> parse_style_path_id(It& it, ItEnd last)
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
[[nodiscard]] constexpr expected_optional<std::string, std::string> parse_style_path_class(It& it, ItEnd last)
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
[[nodiscard]] constexpr expected_optional<length_f, std::string> parse_style_length(It& it, ItEnd last)
{
    hi_assert(it != last);

    if (*it != token::integer and *it != token::floating_point) {
        return std::nullopt
    }

    auto const value = static_cast<float>(*it);
    ++it;

    if (it == last or *it != token::id) {
        // A numeric value without a suffix is in device independet pixels.
        return {name, length_f{dips(value)}};
    } else if (*it == "px") {
        ++it;
        return {name, length_f{pixels(value)}};
    } else if (*it == "dp" or *it == "dip") {
        ++it;
        return {name, length_f{dips(value)}};
    } else if (*it == "pt") {
        ++it;
        return {name, length_f{points(value)}};
    } else if (*it == "in") {
        ++it;
        return {name, length_f{au::inches(value)}};
    } else if (*it == "cm") {
        ++it;
        return {name, length_f{au::centi<au::meters>(value)}};
    } else {
        // Unknown suffix could be token for another part of the tag.
        return {name, length_f{dips(value)}};
    }
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr expected_optional<color, std::string> parse_style_color(It& it, ItEnd last)
{
    hi_assert(it != last);

    if (*it == token::color) {
        ++it;
        return static_cast<color>(it[2]);

    } else if (*it == token::id and *it == "rgb_color") {
        ++it;
        if (it == last or *it != '(') {
            return std::unexpected{std::format("{}: Missing '(' after rgb_color.", token_location(it, last))};
        }

        ++it;
        if (it == last or (*it != token::integer and *it != token::floating_point)) {
            return std::unexpected{
                std::format("{}: Expecting a number as first argument to rgb_color.", token_location(it, last))};
        }
        auto const red = static_cast<float>(*it);

        ++it;
        if (it == last or *it != ',') {
            return std::unexpected{
                std::format("{}: Expecting a comma ',' after first argument to rgb_color.", token_location(it, last))};
        }

        ++it;
        if (it == last or (*it != token::integer and *it != token::floating_point)) {
            return std::unexpected{
                std::format("{}: Expecting a number as second argument to rgb_color.", token_location(it, last))};
        }
        auto const green = static_cast<float>(*it);

        ++it;
        if (it == last or *it != ',') {
            return std::unexpected{
                std::format("{}: Expecting a comma ',' after second argument to rgb_color.", token_location(it, last))};
        }

        ++it;
        if (it == last or (*it != token::integer and *it != token::floating_point)) {
            return std::unexpected{
                std::format("{}: Expecting a number as third argument to rgb_color.", token_location(it, last))};
        }
        auto const blue = static_cast<float>(*it);

        ++it;
        if (it == last or *it != ')') {
            return std::unexpected{std::format("{}: Missing ')' after rgb_color arguments.", token_location(it, last))};
        }

        ++it;
        return color{red, green, blue, 1.0f};

    } else if (*it == token::id and *it == "rgba_color") {
        ++it;
        if (it == last or *it != '(') {
            return std::unexpected{std::format("{}: Missing '(' after rgba_color.", token_location(it, last))};
        }

        ++it;
        if (it == last or (*it != token::integer and *it != token::floating_point)) {
            return std::unexpected{
                std::format("{}: Expecting a number as first argument to rgba_color.", token_location(it, last))};
        }
        auto const red = static_cast<float>(*it);

        ++it;
        if (it == last or *it != ',') {
            return std::unexpected{
                std::format("{}: Expecting a comma ',' after first argument to rgba_color.", token_location(it, last))};
        }

        ++it;
        if (it == last or (*it != token::integer and *it != token::floating_point)) {
            return std::unexpected{
                std::format("{}: Expecting a number as second argument to rgba_color.", token_location(it, last))};
        }
        auto const green = static_cast<float>(*it);

        ++it;
        if (it == last or *it != ',') {
            return std::unexpected{
                std::format("{}: Expecting a comma ',' after second argument to rgba_color.", token_location(it, last))};
        }

        ++it;
        if (it == last or (*it != token::integer and *it != token::floating_point)) {
            return std::unexpected{
                std::format("{}: Expecting a number as third argument to rgba_color.", token_location(it, last))};
        }
        auto const blue = static_cast<float>(*it);

        ++it;
        if (it == last or *it != ',') {
            return std::unexpected{
                std::format("{}: Expecting a comma ',' after third argument to rgba_color.", token_location(it, last))};
        }

        ++it;
        if (it == last or (*it != token::integer and *it != token::floating_point)) {
            return std::unexpected{
                std::format("{}: Expecting a number as forth argument to rgba_color.", token_location(it, last))};
        }
        auto const alpha = static_cast<float>(*it);

        ++it;
        if (it == last or *it != ')') {
            return std::unexpected{std::format("{}: Missing ')' after rgba_color arguments.", token_location(it, last))};
        }

        ++it;
        return color{red, green, blue, alpha};

    } else if (*it == token::id) {
        auto const color_name = static_cast<std::string>(*it);
        ++it;

        if (auto const* color_ptr = color::find(color_name)) {
            return *color_ptr;
        } else {
            return std::unexpected{std::format("{}: Unknown color name '{}'.", token_location(it, last), color_name)};
        }

    } else {
        return std::unexpected{std::format("{}: Unknown color value {}.", token_location(it, last), *it)};
    }
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr expected_optional<style_attributes, std::string> parse_style_attribute(It& it, ItEnd last)
{
#define HIX_VALUE(VALUE_PARSER, NAME, ATTRIBUTE) \
    if (name == NAME) { \
        if (auto const value = VALUE_PARSER(it, last)) { \
            auto r = style_attributes{}; \
            r.set_##ATTRIBUTE(*value, true); \
            return r; \
        } else if (value.has_error()) { \
            return std::unexpected(value.error()); \
        } else { \
            return std::unexpected(std::format("{}: Unknown value {} for attribute '{}'", token_location(it, last), *it, name)); \
        } \
    } else

    hi_assert(it != last);

    if (it[0] != token::id or it + 1 == last or it[1] != '=' or it + 2 == last) {
        return std::nullopt;
    }

    auto const name = static_cast<std::string>(it[0]);
    it += 2;

    HIX_VALUE(parse_style_length, "width", width)
    HIX_VALUE(parse_style_length, "height", height)
    HIX_VALUE(parse_style_length, "margin-left", margin_left)
    HIX_VALUE(parse_style_length, "margin-bottom", margin_bottom)
    HIX_VALUE(parse_style_length, "margin-right", margin_right)
    HIX_VALUE(parse_style_length, "margin-top", margin_top)
    HIX_VALUE(parse_style_length, "margin", margin)
    HIX_VALUE(parse_style_length, "padding-left", padding_left)
    HIX_VALUE(parse_style_length, "padding-bottom", padding_bottom)
    HIX_VALUE(parse_style_length, "padding-right", padding_right)
    HIX_VALUE(parse_style_length, "padding-top", padding_top)
    HIX_VALUE(parse_style_length, "padding", padding)
    HIX_VALUE(parse_style_length, "border-width", border_width)
    HIX_VALUE(parse_style_length, "left-bottom-corner-radius", left_bottom_corner_radius)
    HIX_VALUE(parse_style_length, "right-bottom-corner-radius", right_bottom_corner_radius)
    HIX_VALUE(parse_style_length, "left-top-corner-radius", left_top_corner_radius)
    HIX_VALUE(parse_style_length, "right-top-corner-radius", right_top_corner_radius)
    HIX_VALUE(parse_style_length, "corner-radius", corner_radius)
    HIX_VALUE(parse_style_color, "foreground-color", foreground_color)
    HIX_VALUE(parse_style_color, "background-color", background_color)
    HIX_VALUE(parse_style_color, "border-color", border_color)
    HIX_VALUE(parse_style_horizontal_alignment, "horizontal-alignment", horizontal_alignment)
    HIX_VALUE(parse_style_vertical_alignment, "vertical-alignment", vertical_alignment)
    {
        return std::unexpected(std::format("{}: Unknown attribute '{}'.", token_location(it, last), name));
    }

#undef HIX_VALUE
}

} // namespace detail

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr expected_optional<std::pair<style_attributes, style_path_segment>, std::string> parse_style(It first, ItEnd last)
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

    auto attributes = hi::style_attributes{};
    auto path_segment = hi::style_path_segment{};
    while (token_it != std::sentinel) {
        if (auto attribute = detail::parse_style_attribute(token_it, std::sentinel)) {
            attributes.apply(*attribute);
            continue;

        } else if (attribute.has_error()) {
            return std::unexpected{attribute.error()};
        }

        if (auto id = detail::parse_style_path_id(token_it, std::sentinel)) {
            if (path_segment.id.empty()) {
                path_segment.id = *id;
                continue;
            } else {
                return std::unexpected{std::format("{}: Style already has id #{}.", token_location(token_it, std::sentinel), path_segment.id)};
            }

        } else if (id.has_error()) {
            return std::unexpected{id.error()};
        }

        if (auto class_ = detail::parse_style_path_class(token_it, std::sentinel)) {
            path_segment.classes.push_back(*class_);
            continue;

        } else if (class_.has_error()) {
            return std::unexpected{class_.error()};
        }

        return std::unexpected{std::format("{}: Unexpected token '{}'.", token_location(token_it, std::sentinel), *token_it)};
    }

    return {attributes, path_segment};
}

} // namespace v1
} // namespace hi::v1
