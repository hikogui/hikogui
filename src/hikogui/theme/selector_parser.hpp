

#pragma once

#include "../parser/parser.hpp"
#include "../macros.hpp"
#include <iterator>

namespace hi { inline namespace v1 {

[[nodiscard]] constexpr static lexer_config theme_selector_lexer_config() noexcept
{
    auto r = lexer_config{};
    r.has_double_quote_string_literal = 1;
    r.has_color_literal = 1;
    r.filter_white_space = 1;
    r.minus_in_identifier = 1;
    return r;
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::parse_expected<theme_selector_segment, std::string> parse_selector_segment(It &it, ItEnd last)
{
    hi_assert(it != last);

    if (it[0] != '/') {
        return std::nullopt;
    }

    if (it == last or it[1] != token::id) {
        return std::unexpected{std::format("{}: Expected a widg
        et name after '/', got '{}'.", token_location(it + 1, last), it[1])};
    }

    auto r = theme_selector_segment{static_cast<std::string>(it[1])};

    it += 2;
    while (it != last and *it != '/') {
        if (auto id = parse_selector_id(it, last)) {
            r.id = *id;
            continue;
        } else if (id.has_error()) {
            return std::unexpected{id.error()};
        }

        if (auto class_name = parse_selector_class(it, last)) {
            r.classes.push_back(*class_name);
            continue;
        } else if (class_name.has_error()) {
            return std::unexpected{class_name.error()};
        }

        if (auto attribute = parse_selector_attribute(it, last)) {
            r.attributes.push_back(*attribute);
            continue;
        } else if (attribute.has_error()) {
            return std::unexpected{attribute.error()};
        }
    }

    return r;
}

template<std::forward_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::expected<theme_selector, std::string> parse_selector(It first, ItEnd last)
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

    auto r = std::vector<theme_selector_segment>{}
    while (token_it != std::sentinel) {
        if (auto segment = parse_selector_segment(token_it, std::sentinel)) {
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

    return theme_selector{std::move(r)};
}

}} // namespace hi::v1
