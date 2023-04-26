// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../color/module.hpp"
#include "../font/module.hpp"
#include "../text/module.hpp"
#include "../i18n/module.hpp"
#include "theme_mode.hpp"
#include "theme_state.hpp"
#include "theme_length.hpp"
#include "theme_model.hpp"
#include <string>
#include <vector>
#include <variant>

namespace hi { inline namespace v1 {


enum class style_sheet_value_mask {
    pixels = 0b0000'0001,
    dips = 0b0000'0010,
    em_quads = 0b0000'0100,
    color = 0b0000'1000,
    font_family_id = 0b0001'0000,
    font_weight = 0b0010'0000,
    font_style = 0b0100'0000,

    length = pixels | dips | em_quads,
};

[[nodiscard]] constexpr style_sheet_value_mask
operator|(style_sheet_value_mask const& lhs, style_sheet_value_mask const& rhs) noexcept
{
    return static_cast<style_sheet_value_mask>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr style_sheet_value_mask
operator&(style_sheet_value_mask const& lhs, style_sheet_value_mask const& rhs) noexcept
{
    return static_cast<style_sheet_value_mask>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr bool to_bool(style_sheet_value_mask const& rhs) noexcept
{
    return static_cast<bool>(to_underlying(rhs));
}

struct style_sheet_value :
    std::variant<hi::pixels, hi::dips, hi::em_quads, hi::color, font_family_id, font_weight, font_style> {
    using super = std::variant<hi::pixels, hi::dips, hi::em_quads, hi::color, font_family_id, font_weight, font_style>;
    using super::super;

    constexpr style_sheet_value(theme_length length) noexcept :
        super([&] {
            if (auto pixels_ptr = std::get_if<hi::pixels>(&length)) {
                return super{*pixels_ptr};
            } else if (auto dips_ptr = std::get_if<hi::dips>(&length)) {
                return super{*dips_ptr};
            } else if (auto em_quads_ptr = std::get_if<hi::em_quads>(&length)) {
                return super{*em_quads_ptr};
            } else {
                hi_no_default();
            }
        }())
    {
    }
};

struct style_sheet_pattern {
    std::vector<std::string> path;
    std::vector<bool> is_child;

    [[nodiscard]] constexpr std::string get_path_as_string() const noexcept
    {
        hi_assert(not path.empty());
        auto r = path[0];

        hi_assert(path.size() == is_child.size() + 1);

        for (auto i = 0_uz; i != is_child.size(); ++i) {
            if (is_child[i]) {
                r += ">";
            } else {
                r += " ";
            }
            r += path[i + 1];
        }

        return r;
    }
};

struct style_sheet_selector : std::vector<style_sheet_pattern> {};

enum class style_sheet_declaration_name {
    background_color,
    border_bottom_left_radius,
    border_bottom_right_radius,
    border_color,
    border_top_left_radius,
    border_top_right_radius,
    border_width,
    caret_color_primary,
    caret_color_secondary,
    fill_color,
    font_color,
    font_family,
    font_size,
    font_style,
    font_weight,
    height,
    margin_bottom,
    margin_left,
    margin_right,
    margin_top,
    selection_color,
    spacing_horizontal,
    spacing_vertical,
    width,
};

// clang-format off
constexpr auto style_sheet_declaration_name_metadata = enum_metadata{
    style_sheet_declaration_name::background_color, "background-color",
    style_sheet_declaration_name::border_bottom_left_radius, "border-bottom-left-radius",
    style_sheet_declaration_name::border_bottom_right_radius, "border-bottom-right-radius",
    style_sheet_declaration_name::border_color, "border-color",
    style_sheet_declaration_name::border_top_left_radius, "border-top-left-radius",
    style_sheet_declaration_name::border_top_right_radius, "border-top-right-radius",
    style_sheet_declaration_name::border_width, "border-width",
    style_sheet_declaration_name::caret_color_primary, "caret-color-primary",
    style_sheet_declaration_name::caret_color_secondary, "caret-color-secondary",
    style_sheet_declaration_name::fill_color, "fill-color",
    style_sheet_declaration_name::font_color, "font-color",
    style_sheet_declaration_name::font_family, "font-family",
    style_sheet_declaration_name::font_size, "font-size",
    style_sheet_declaration_name::font_style, "font-style",
    style_sheet_declaration_name::font_weight, "font-weight",
    style_sheet_declaration_name::height, "height",
    style_sheet_declaration_name::margin_bottom, "margin-bottom",
    style_sheet_declaration_name::margin_left, "margin-left",
    style_sheet_declaration_name::margin_right, "margin-right",
    style_sheet_declaration_name::margin_top, "margin-top",
    style_sheet_declaration_name::selection_color, "selection-color",
    style_sheet_declaration_name::spacing_horizontal, "spacing-horizontal",
    style_sheet_declaration_name::spacing_vertical, "spacing-vertical",
    style_sheet_declaration_name::width, "width",
};
// clang-format on

// clang-format off
constexpr auto style_sheet_declaration_name_value_mask_metadata = enum_metadata{
    style_sheet_declaration_name::background_color, style_sheet_value_mask::color,
    style_sheet_declaration_name::border_bottom_left_radius, style_sheet_value_mask::length,
    style_sheet_declaration_name::border_bottom_right_radius, style_sheet_value_mask::length,
    style_sheet_declaration_name::border_color, style_sheet_value_mask::color,
    style_sheet_declaration_name::border_top_left_radius, style_sheet_value_mask::length,
    style_sheet_declaration_name::border_top_right_radius, style_sheet_value_mask::length,
    style_sheet_declaration_name::border_width, style_sheet_value_mask::length,
    style_sheet_declaration_name::caret_color_primary, style_sheet_value_mask::color,
    style_sheet_declaration_name::caret_color_secondary, style_sheet_value_mask::color,
    style_sheet_declaration_name::fill_color, style_sheet_value_mask::color,
    style_sheet_declaration_name::font_color, style_sheet_value_mask::color,
    style_sheet_declaration_name::font_family, style_sheet_value_mask::font_family_id,
    style_sheet_declaration_name::font_size, style_sheet_value_mask::dips,
    style_sheet_declaration_name::font_style, style_sheet_value_mask::font_style,
    style_sheet_declaration_name::font_weight, style_sheet_value_mask::font_weight,
    style_sheet_declaration_name::height, style_sheet_value_mask::length,
    style_sheet_declaration_name::margin_bottom, style_sheet_value_mask::length,
    style_sheet_declaration_name::margin_left, style_sheet_value_mask::length,
    style_sheet_declaration_name::margin_right, style_sheet_value_mask::length,
    style_sheet_declaration_name::margin_top, style_sheet_value_mask::length,
    style_sheet_declaration_name::selection_color, style_sheet_value_mask::color,
    style_sheet_declaration_name::spacing_horizontal, style_sheet_value_mask::length,
    style_sheet_declaration_name::spacing_vertical, style_sheet_value_mask::length,
    style_sheet_declaration_name::width, style_sheet_value_mask::length,
};
// clang-format on

struct style_sheet_declaration {
    style_sheet_declaration_name name;
    style_sheet_value value;
    bool important = false;
};

struct style_sheet_rule_set {
    style_sheet_selector selector;
    theme_state state = {};
    theme_state_mask state_mask = {};
    text_phrasing_mask phrasing_mask = {};
    language_tag language = {};

    std::vector<style_sheet_declaration> declarations;

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return declarations.size();
    }

    [[nodiscard]] constexpr style_sheet_declaration const& operator[](size_t i) const noexcept
    {
        return declarations[i];
    }

    [[nodiscard]] constexpr std::string get_selector_as_string() const noexcept
    {
        hi_assert(not selector.empty());

        auto r = selector[0].get_path_as_string();
        for (auto i = 1_uz; i != selector.size(); ++i) {
            r += ',';
            r += selector[i].get_path_as_string();
        }

        return r;
    }
};

struct style_sheet {
    std::string name;
    theme_mode mode;

    std::vector<std::pair<std::string, color>> colors;
    std::vector<style_sheet_rule_set> rule_sets;

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return rule_sets.size();
    }

    [[nodiscard]] style_sheet_rule_set const& operator[](size_t i) const noexcept
    {
        return rule_sets[i];
    }

    /** Activate style sheet as the current theme.
    */
    void activate() const noexcept
    {
    }
};

}} // namespace hi::v1
