// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "style_selector.hpp"
#include "style_properties.hpp"
#include "style_pseudo_class.hpp"
#include "style_path.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"


hi_export_module(hikogui.theme : style_cascade);

hi_export namespace hi::inline v1 {
namespace detail {

struct style_property_element {
    style_selector selector;
    style_properties properties;
};

[[nodiscard]] inline auto initial_style_properties_init() noexcept
{
    auto r = std::vector<style_property_element>{};

    {
        // Match all widgets with the :root pseudo class.
        // Set actual properties to make the GUI work without a style sheet.
        auto selector = style_selector{};
        selector.emplace_back(std::vector<std::string>{}, std::vector<std::string>{"root"});

        auto const priority = style_priority{style_importance::initial, selector.specificity()};

        auto properties = style_properties{};
        properties.set_width(unit::pixels(20.0f), priority);
        properties.set_height(unit::pixels(20.0f), priority);
        properties.set_font_size(unit::pixels_per_em(15.0f), priority);
        properties.set_margin_left(unit::pixels(5.0f), priority);
        properties.set_margin_bottom(unit::pixels(5.0f), priority);
        properties.set_margin_right(unit::pixels(5.0f), priority);
        properties.set_margin_top(unit::pixels(5.0f), priority);
        properties.set_padding_left(unit::pixels(5.0f), priority);
        properties.set_padding_bottom(unit::pixels(5.0f), priority);
        properties.set_padding_right(unit::pixels(5.0f), priority);
        properties.set_padding_top(unit::pixels(5.0f), priority);
        properties.set_border_width(unit::pixels(1.0f), priority);
        properties.set_border_bottom_left_radius(unit::pixels(0.0f), priority);
        properties.set_border_bottom_right_radius(unit::pixels(0.0f), priority);
        properties.set_border_top_left_radius(unit::pixels(0.0f), priority);
        properties.set_border_top_right_radius(unit::pixels(0.0f), priority);
        properties.set_x_height(unit::pixels(10.0f), priority);
        properties.set_horizontal_alignment(horizontal_alignment::left, priority);
        properties.set_vertical_alignment(vertical_alignment::middle, priority);
        properties.set_foreground_color(color{0.0f, 0.0f, 0.0f, 1.0f}, priority);
        properties.set_background_color(color{1.0f, 1.0f, 1.0f, 1.0f}, priority);
        properties.set_border_color(color{0.0f, 0.0f, 0.0f, 1.0f}, priority);
        properties.set_accent_color(color{0.0f, 0.0f, 1.0f, 1.0f}, priority);

        auto text_styles = text_style_set{};
        auto text_style = hi::text_style{};

        auto font_chain = lean_vector<font_id>{};
        if (auto font = find_font("Arial")) {
            font_chain.push_back(font);
        }
        if (auto font = find_font("Helvetica")) {
            font_chain.push_back(font);
        }

        text_style.set_font_chain(std::move(font_chain));
        text_style.set_scale(1.0f);
        text_style.set_color(color{0.0f, 0.0f, 0.0f, 1.0f});
        text_style.set_line_spacing(1.0f);
        text_style.set_paragraph_spacing(1.5f);
        text_styles.push_back(grapheme_attribute_mask{}, text_style);
        properties.set_text_style(text_styles, priority);

        r.emplace_back(std::move(selector), std::move(properties));
    }

    return r;
}

auto initial_style_properties = std::vector<style_property_element>{};
auto user_style_properties = std::vector<style_property_element>{};
auto theme_style_properties = std::vector<style_property_element>{};
auto author_style_properties = std::vector<style_property_element>{};


constexpr void generate_pseudo_classes_from_nesting_depth(size_t depth, std::vector<std::string>& r)
{
    if (depth == 0) {
        r.emplace_back("root");
    }

    if (depth % 2 == 0) {
        r.emplace_back("nth-depth(odd)");
    } else {
        r.emplace_back("nth-depth(even)");
    }

    for (size_t i = 2; i != 11; ++i) {
        auto const j = (depth + 1) % i;
        if (j == 0) {
            r.push_back(std::format("nth-depth({}n)", i));
        } else {
            r.push_back(std::format("nth-depth({}n+{})", i, j));
            r.push_back(std::format("nth-depth({}n-{})", i, i - j));
        }
    }
}

constexpr void generate_pseudo_classes_from_enum(style_pseudo_class pseudo_class, std::vector<std::string>& r)
{
    switch(pseudo_class & style_pseudo_class::mode_mask) {
    case style_pseudo_class::disabled:
        r.emplace_back("disabled");
        break;
    case style_pseudo_class::enabled:
        r.emplace_back("enabled");
        break;
    case style_pseudo_class::hover:
        r.emplace_back("enabled");
        r.emplace_back("hover");
        break;
    case style_pseudo_class::active:
        r.emplace_back("enabled");
        r.emplace_back("hover");
        r.emplace_back("active");
        break;
    default:
        std::unreachable();
    }

    if (std::to_underlying(pseudo_class & style_pseudo_class::focus)) {
        r.emplace_back("focus");
    }

    if (std::to_underlying(pseudo_class & style_pseudo_class::checked)) {
        r.emplace_back("checked");
    }

    if (std::to_underlying(pseudo_class & style_pseudo_class::front)) {
        r.emplace_back("front");
    }
}

[[nodiscard]] constexpr std::vector<std::string> generate_pseudo_classes(size_t depth, style_pseudo_class pseudo_class)
{
    auto r = std::vector<std::string>{};
    generate_pseudo_classes_from_enum(pseudo_class, r);
    generate_pseudo_classes_from_nesting_depth(depth, r);
    std::sort(r.begin(), r.end());
    return r;
}

}

/**
 * @brief Resets the style properties based on the specified importance.
 * 
 * This function clears the style properties based on the specified importance.
 * The importance can be one of the following values:
 * - style_importance::initial: Clears the initial style properties.
 * - style_importance::user: Clears the user-defined style properties.
 * - style_importance::theme: Clears the theme-defined style properties.
 * - style_importance::author: Clears the author-defined style properties.
 * 
 * @param importance The importance of the style properties to be cleared.
 */
inline void reset_style_properties(style_importance importance) noexcept
{
    switch (importance) {
    case style_importance::initial:
        detail::initial_style_properties.clear();
        break;
    case style_importance::user:
        detail::user_style_properties.clear();
        break;
    case style_importance::theme:
        detail::theme_style_properties.clear();
        break;
    case style_importance::author:
        detail::author_style_properties.clear();
        break;
    default:
        std::unreachable();
    }
}

/**
 * Adds style properties to the appropriate part of the cascade based on the given importance.
 *
 * @param importance The importance level of the style properties.
 * @param selector The selector for the style properties.
 * @param properties The style properties to be added.
 */
inline void add_style_properties(style_importance importance, style_selector selector, style_properties properties)
{
    switch (importance) {
    case style_importance::initial:
        detail::initial_style_properties.emplace_back(std::move(selector), std::move(properties));
        break;
    case style_importance::user:
        detail::user_style_properties.emplace_back(std::move(selector), std::move(properties));
        break;
    case style_importance::theme:
        detail::theme_style_properties.emplace_back(std::move(selector), std::move(properties));
        break;
    case style_importance::author:
        detail::author_style_properties.emplace_back(std::move(selector), std::move(properties));
        break;
    default:
        std::unreachable();
    }
}

/**
 * Returns a generator that yields all style properties.
 *
 * This function returns a generator that yields all style properties in the following order:
 * 1. Initial style properties
 * 2. User-defined style properties
 * 3. Theme-defined style properties
 * 4. Author-defined style properties
 *
 * @return A generator that yields all style properties.
 */
inline generator<detail::style_property_element const&> all_style_properties() noexcept
{
    // Load in the initial style properties lazily.
    // This is done to avoid static initialization order fiasco.
    // Specifically the font-book must be initialized first.
    if (detail::initial_style_properties.empty()) {
        detail::initial_style_properties = detail::initial_style_properties_init();
    }

    for (auto const& element : detail::initial_style_properties) {
        co_yield element;
    }
    for (auto const& element : detail::user_style_properties) {
        co_yield element;
    }
    for (auto const& element : detail::theme_style_properties) {
        co_yield element;
    }
    for (auto const& element : detail::author_style_properties) {
        co_yield element;
    }
}

/**
 * Retrieves the style properties for a given style path and pseudo class.
 *
 * @param path The style path to match against.
 * @param pseudo_class The pseudo class to match against.
 * @return The style properties that match the given style path and pseudo class.
 */
[[nodiscard]] inline style_properties get_style_properties(style_path const& path, style_pseudo_class pseudo_class) noexcept
{
    auto const pseudo_classes = detail::generate_pseudo_classes(path.nesting_depth(), pseudo_class);

    auto r = style_properties{};

    for (auto const& element : all_style_properties()) {
        if (matches(element.selector, path, pseudo_classes)) {
            r.apply(element.properties); 
        }
    }

    return r;
}

}

