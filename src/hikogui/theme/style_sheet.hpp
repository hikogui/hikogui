// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../color/module.hpp"
#include "../font/module.hpp"
#include <string>
#include <vector>
#include <variant>

namespace hi { inline namespace v1 {

struct style_sheet_length : std::variant<hi::pixels, hi::points, hi::em_quads> {
    using super = std::variant<hi::pixels, hi::points, hi::em_quads>;
    using super::super;
};

struct style_sheet_value :
    std::variant<hi::pixels, hi::points, hi::em_quads, hi::color, font_family_id, font_weight, font_style> {
    using super = std::variant<hi::pixels, hi::points, hi::em_quads, hi::color, font_family_id, font_weight, font_style>;
    using super::super;

    constexpr style_sheet_value(style_sheet_length length) noexcept :
        super([&] {
            if (auto pixels_ptr = std::get_if<hi::pixels>(&length)) {
                return super{*pixels_ptr};
            } else if (auto points_ptr = std::get_if<hi::points>(&length)) {
                return super{*points_ptr};
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
    std::vector<std::string> states;

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

struct style_sheet_declaration {
    style_sheet_declaration_name name;
    style_sheet_value value;
    bool important = false;
};

struct style_sheet_rule_set {
    style_sheet_selector selector;
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
};

}} // namespace hi::v1
