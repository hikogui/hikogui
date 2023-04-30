// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../color/module.hpp"
#include "../font/module.hpp"
#include "../text/module.hpp"
#include "../i18n/module.hpp"
#include "../file/module.hpp"
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

struct style_sheet_value : std::variant<hi::pixels, hi::dips, hi::em_quads, hi::color, font_family_id, font_weight, font_style> {
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

    mutable glob_pattern pattern_cache = {};
    mutable bool pattern_cache_valid = false;

    [[nodiscard]] constexpr bool matches(std::string_view model_path) const noexcept
    {
        if (not pattern_cache_valid) {
            pattern_cache = get_path_as_glob_pattern();
            pattern_cache_valid = true;
        }
        return pattern_cache.matches(model_path);
    }

    [[nodiscard]] constexpr std::string get_path_as_glob_string() const noexcept
    {
        hi_assert(not path.empty());

        auto r = path.front();

        hi_assert(path.size() == is_child.size() + 1);

        for (auto i = 0_uz; i != is_child.size(); ++i) {
            r += is_child[i] ? "/" : "/**/";
            r += path[i + 1];
        }

        return r;
    }

    [[nodiscard]] constexpr glob_pattern get_path_as_glob_pattern() const noexcept
    {
        return glob_pattern{get_path_as_glob_string()};
    }
};

struct style_sheet_selector : std::vector<style_sheet_pattern> {
    [[nodiscard]] bool matches(std::string_view path) const noexcept
    {
        for (hilet& pattern : *this) {
            if (pattern.matches(path)) {
                return true;
            }
        }
        return false;
    }
};

enum class style_sheet_declaration_name {
    background_color,
    border_bottom_left_radius,
    border_bottom_right_radius,
    border_color,
    border_top_left_radius,
    border_top_right_radius,
    border_width,
    caret_primary_color,
    caret_secondary_color,
    caret_overwrite_color,
    caret_compose_color,
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
    style_sheet_declaration_name::caret_primary_color, "caret-primary-color",
    style_sheet_declaration_name::caret_secondary_color, "caret-secondary-color",
    style_sheet_declaration_name::caret_overwrite_color, "caret-overwrite-color",
    style_sheet_declaration_name::caret_compose_color, "caret-compose-color",
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
    style_sheet_declaration_name::caret_primary_color, style_sheet_value_mask::color,
    style_sheet_declaration_name::caret_secondary_color, style_sheet_value_mask::color,
    style_sheet_declaration_name::caret_overwrite_color, style_sheet_value_mask::color,
    style_sheet_declaration_name::caret_compose_color, style_sheet_value_mask::color,
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
    language_tag language_mask = {};

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

        auto r = selector[0].get_path_as_glob_string();
        for (auto i = 1_uz; i != selector.size(); ++i) {
            r += ',';
            r += selector[i].get_path_as_glob_string();
        }

        return r;
    }

    [[nodiscard]] generator<theme_state> matching_states(std::string const& model_path) const noexcept
    {
        if (selector.matches(model_path)) {
            for (auto i = theme_state{}; i != theme_state{theme_state_size}; ++i) {
                if ((i & state_mask) == state) {
                    co_yield i;
                }
            }
        }
    }

    void activate_model(int phase, std::string const& model_path, theme_model_base& model) const noexcept
    {
        for (auto model_state : matching_states(model_path)) {
            auto& sub_model = model[model_state];
            for (hilet & [ name, value, important ] : declarations) {
                _activate_model_declaration(phase, model_path, sub_model, name, value, important);
            }
        }
    }

private:
    void _activate_model_font_color(int phase, theme_sub_model& sub_model, style_sheet_value value) const noexcept
    {
        if (phase != 0) {
            return;
        }

        auto& text_style = sub_model.text_theme.find_or_add(phrasing_mask, language_mask);
        hi_axiom(std::holds_alternative<hi::color>(value));
        text_style.color = std::get<hi::color>(value);
    }

    void _activate_model_font_family(int phase, theme_sub_model& sub_model, style_sheet_value value) const noexcept
    {
        if (phase != 0) {
            return;
        }

        auto& text_style = sub_model.text_theme.find_or_add(phrasing_mask, language_mask);
        hi_axiom(std::holds_alternative<hi::font_family_id>(value));
        text_style.family_id = std::get<hi::font_family_id>(value);
    }

    void _activate_model_font_style(int phase, theme_sub_model& sub_model, style_sheet_value value) const noexcept
    {
        if (phase != 0) {
            return;
        }

        auto& text_style = sub_model.text_theme.find_or_add(phrasing_mask, language_mask);
        hi_axiom(std::holds_alternative<hi::font_style>(value));
        text_style.variant.set_style(std::get<hi::font_style>(value));
    }

    void _activate_model_font_size(int phase, theme_sub_model& sub_model, style_sheet_value value) const noexcept
    {
        if (phase != 0) {
            return;
        }

        auto& text_style = sub_model.text_theme.find_or_add(phrasing_mask, language_mask);
        hi_axiom(std::holds_alternative<hi::dips>(value));

        // When retrieving the text-style it will be scaled by the UI-scale.
        text_style.size = round_cast<int>(std::get<hi::dips>(value).count() * -4.0);

        if (not language_mask and not to_bool(phrasing_mask)) {
            sub_model.font_line_height = std::get<hi::dips>(value);
            // The following values are estimates.
            // Hopefully good enough for calculating baselines and such.
            // We could not get proper sizes anyway since there may be multiple
            // fonts defined in the test_theme.
            sub_model.font_cap_height = std::get<hi::dips>(value) * 0.7;
            sub_model.font_x_height = std::get<hi::dips>(value) * 0.48;
        }
    }

    void _activate_model_font_weight(int phase, theme_sub_model& sub_model, style_sheet_value value) const noexcept
    {
        if (phase != 0) {
            return;
        }

        auto& text_style = sub_model.text_theme.find_or_add(phrasing_mask, language_mask);
        hi_axiom(std::holds_alternative<hi::font_weight>(value));
        text_style.variant.set_weight(std::get<hi::font_weight>(value));
    }

    template<style_sheet_declaration_name Name>
    void _activate_model_color(int phase, theme_sub_model& sub_model, style_sheet_value value, bool important) const noexcept
    {
        hi_no_default();
    }

    template<style_sheet_declaration_name Name>
    void _activate_model_length(int phase, theme_sub_model& sub_model, style_sheet_value value, bool important) const noexcept
    {
        hi_no_default();
    }

#define HI_X_COLOR_VALUE(NAME) \
    template<> \
    hi_no_inline void _activate_model_color<style_sheet_declaration_name::NAME>( \
        int phase, theme_sub_model& sub_model, style_sheet_value value, bool important) const noexcept \
    { \
        if (phase != 1) { \
            return; \
        } \
\
        if (not sub_model.NAME##_important or important) { \
            sub_model.NAME = std::get<hi::color>(value); \
        } \
        sub_model.NAME##_important |= important; \
        sub_model.NAME##_assigned = 1; \
    }

    HI_X_COLOR_VALUE(background_color)
    HI_X_COLOR_VALUE(border_color)
    HI_X_COLOR_VALUE(fill_color)
    HI_X_COLOR_VALUE(selection_color)
    HI_X_COLOR_VALUE(caret_primary_color)
    HI_X_COLOR_VALUE(caret_secondary_color)
    HI_X_COLOR_VALUE(caret_overwrite_color)
    HI_X_COLOR_VALUE(caret_compose_color)
#undef HI_X_COLOR_VALUE

#define HI_X_LENGTH_VALUE(NAME) \
    template<> \
    hi_no_inline void _activate_model_length<style_sheet_declaration_name::NAME>( \
        int phase, theme_sub_model& sub_model, style_sheet_value value, bool important) const noexcept \
    { \
        if (phase != 1) { \
            return; \
        } \
\
        if (not sub_model.NAME##_important or important) { \
            if (hilet dp_ptr = std::get_if<hi::dips>(&value)) { \
                sub_model.NAME = *dp_ptr; \
            } else if (hilet px_ptr = std::get_if<hi::pixels>(&value)) { \
                sub_model.NAME = *px_ptr; \
            } else if (hilet em_ptr = std::get_if<hi::em_quads>(&value)) { \
                sub_model.NAME = static_cast<hi::dips>(sub_model.font_line_height) * em_ptr->count(); \
            } else { \
                hi_no_default(); \
            } \
        } \
        sub_model.NAME##_important |= important; \
        sub_model.NAME##_assigned = 1; \
    }

    HI_X_LENGTH_VALUE(width)
    HI_X_LENGTH_VALUE(height)
    HI_X_LENGTH_VALUE(border_width)
    HI_X_LENGTH_VALUE(border_bottom_left_radius)
    HI_X_LENGTH_VALUE(border_bottom_right_radius)
    HI_X_LENGTH_VALUE(border_top_left_radius)
    HI_X_LENGTH_VALUE(border_top_right_radius)
    HI_X_LENGTH_VALUE(margin_left)
    HI_X_LENGTH_VALUE(margin_bottom)
    HI_X_LENGTH_VALUE(margin_right)
    HI_X_LENGTH_VALUE(margin_top)
    HI_X_LENGTH_VALUE(spacing_horizontal)
    HI_X_LENGTH_VALUE(spacing_vertical)
#undef HI_X_LENGTH_VALUE

    void _activate_model_declaration(
        int phase,
        std::string const& model_path,
        theme_sub_model& sub_model,
        style_sheet_declaration_name name,
        style_sheet_value value,
        bool important) const noexcept
    {
        using enum style_sheet_declaration_name;

        switch (name) {
        case background_color:
            return _activate_model_color<background_color>(phase, sub_model, value, important);
        case border_bottom_left_radius:
            return _activate_model_length<border_bottom_left_radius>(phase, sub_model, value, important);
        case border_bottom_right_radius:
            return _activate_model_length<border_bottom_right_radius>(phase, sub_model, value, important);
        case border_color:
            return _activate_model_color<border_color>(phase, sub_model, value, important);
        case border_top_left_radius:
            return _activate_model_length<border_top_left_radius>(phase, sub_model, value, important);
        case border_top_right_radius:
            return _activate_model_length<border_top_right_radius>(phase, sub_model, value, important);
        case border_width:
            return _activate_model_length<border_width>(phase, sub_model, value, important);
        case caret_primary_color:
            return _activate_model_color<caret_primary_color>(phase, sub_model, value, important);
        case caret_secondary_color:
            return _activate_model_color<caret_secondary_color>(phase, sub_model, value, important);
        case caret_overwrite_color:
            return _activate_model_color<caret_overwrite_color>(phase, sub_model, value, important);
        case caret_compose_color:
            return _activate_model_color<caret_compose_color>(phase, sub_model, value, important);
        case fill_color:
            return _activate_model_color<fill_color>(phase, sub_model, value, important);
        case font_color:
            return _activate_model_font_color(phase, sub_model, value);
        case font_family:
            return _activate_model_font_family(phase, sub_model, value);
        case font_size:
            return _activate_model_font_size(phase, sub_model, value);
        case font_style:
            return _activate_model_font_style(phase, sub_model, value);
        case font_weight:
            return _activate_model_font_weight(phase, sub_model, value);
        case height:
            return _activate_model_length<height>(phase, sub_model, value, important);
        case margin_bottom:
            return _activate_model_length<margin_bottom>(phase, sub_model, value, important);
        case margin_left:
            return _activate_model_length<margin_left>(phase, sub_model, value, important);
        case margin_right:
            return _activate_model_length<margin_right>(phase, sub_model, value, important);
        case margin_top:
            return _activate_model_length<margin_top>(phase, sub_model, value, important);
        case selection_color:
            return _activate_model_color<selection_color>(phase, sub_model, value, important);
        case spacing_horizontal:
            return _activate_model_length<spacing_horizontal>(phase, sub_model, value, important);
        case spacing_vertical:
            return _activate_model_length<spacing_vertical>(phase, sub_model, value, important);
        case width:
            return _activate_model_length<width>(phase, sub_model, value, important);
        default:
            hi_no_default();
        }
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
        // First activate the font-styles, so that the size of the font can be
        // used to calculate the size of the other lengths.
        _activate_colors();
        _activate(0);
        _activate(1);
    }

private:
    void _activate_colors() const noexcept
    {
        for (hilet& color_name : color::list()) {
            hilet it = std::find_if(colors.cbegin(), colors.cend(), [&color_name](hilet& x) {
                return x.first == color_name;
            });

            if (it != colors.end()) {
                hilet color_ptr = color::find(color_name);
                hi_axiom_not_null(color_ptr);
                *color_ptr = it->second;

                hi_log_info("Named color '{}' assigned value by theme '{}:{}'", color_name, name, mode);
            } else {
                hi_log_error("Named color '{}' not declared in theme '{}:{}'", color_name, name, mode);
            }
        }
    }

    void _activate(int phase) const noexcept
    {
        for (hilet& model_path : theme_model_keys()) {
            auto& model = theme_model_by_key(model_path);

            for (hilet& rule_set : rule_sets) {
                rule_set.activate_model(phase, model_path, model);
            }
        }
    }
};

}} // namespace hi::v1
