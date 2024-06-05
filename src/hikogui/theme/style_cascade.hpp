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

[[nodiscard]] constexpr auto initial_style_properties_init() noexcept
{
    auto r = std::vector<style_property_element>{};

    return r;
}

auto initial_style_properties = initial_style_properties_init();
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

