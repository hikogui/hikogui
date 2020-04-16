// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Text/TextStyle.hpp"
#include "TTauri/GUI/ThemeMode.hpp"
#include <array>

namespace TTauri::GUI {


class SubTheme {
    std::vector<vec> colors;
    std::vector<vec> fillColors;
    std::vector<vec> borderColors;

    std::array<vec,11> grayColors;

    std::vector<Text::TextStyle> labelStyles;

public:
    Text::TextStyle warningLabelStyle;
    Text::TextStyle errorLabelStyle;
    Text::TextStyle helpLabelStyle;
    Text::TextStyle linkLabelStyle;

    // Themed bright colors.
    vec blueColor;
    vec greenColor;
    vec indigoColor;
    vec orangeColor;
    vec pinkColor;
    vec purpleColor;
    vec redColor;
    vec tealColor;
    vec yellowColor;

    // Semantic colors
    vec accentColor;
    vec keyboardFocusColor;
    vec textSelectColor;
    vec cursorColor;
    vec incompleteGlyphColor;

    /** Get color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec color(int nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        ttauri_assume(ssize(colors) > 0);
        return colors[nestingLevel % ssize(colors)];
    }

    /** Get fill color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec fillColor(int nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        ttauri_assume(ssize(fillColors) > 0);
        return fillColors[nestingLevel % ssize(fillColors)];
    }

    /** Get border color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec borderColor(int nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        ttauri_assume(ssize(borderColors) > 0);
        return borderColors[nestingLevel % ssize(borderColors)];
    }

    /** Get grey scale color
     * This color is reversed between light and dark themes.
     * @param level Color 5 foreground, 0 mid-gray, -5 background
     */
    [[nodiscard]] vec grayColor(int level) const noexcept {
        constexpr int maxLevel = static_cast<int>(std::tuple_size_v<decltype(grayColors)>) / 2;
        constexpr int minLevel = -maxLevel;
        int level_ = std::clamp(minLevel, level, maxLevel) + maxLevel;
        return borderColors[level_];
    }


    /** Get border color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] Text::TextStyle const &labelStyle(int nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        ttauri_assume(ssize(labelStyles) > 0);
        return labelStyles[nestingLevel % ssize(labelStyles)];
    }
};

class Theme {
private:
    std::array<SubTheme,4> subThemes;

    SubTheme const &subTheme(ThemeMode mode) const noexcept {
        return subThemes[static_cast<int>(mode)];
    }

public:

    /** Get color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec color(int nestingLevel) const noexcept {
        return subTheme(themeMode).color(nestingLevel);
    }

    /** Get fill color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec fillColor(int nestingLevel) const noexcept {
        return subTheme(themeMode).fillColor(nestingLevel);
    }

    /** Get border color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec borderColor(int nestingLevel) const noexcept {
        return subTheme(themeMode).borderColor(nestingLevel);
    }

    /** Get grey scale color
     * This color is reversed between light and dark themes.
     * @param level Color 5 foreground, 0 mid-gray, -5 background
     */
    [[nodiscard]] vec grayColor(int level) const noexcept {
        return subTheme(themeMode).grayColor(level);
    }


    [[nodiscard]] vec blueColor() const noexcept {
        return subTheme(themeMode).blueColor;
    }

    [[nodiscard]] vec greenColor() const noexcept {
        return subTheme(themeMode).greenColor;
    }

    [[nodiscard]] vec indigoColor() const noexcept {
        return subTheme(themeMode).indigoColor;
    }

    [[nodiscard]] vec orangeColor() const noexcept {
        return subTheme(themeMode).orangeColor;
    }

    [[nodiscard]] vec pinkColor() const noexcept {
        return subTheme(themeMode).pinkColor;
    }

    [[nodiscard]] vec purpleColor() const noexcept {
        return subTheme(themeMode).purpleColor;
    }

    [[nodiscard]] vec redColor() const noexcept {
        return subTheme(themeMode).redColor;
    }

    [[nodiscard]] vec tealColor() const noexcept {
        return subTheme(themeMode).tealColor;
    }

    [[nodiscard]] vec yellowColor() const noexcept {
        return subTheme(themeMode).yellowColor;
    }



    /** Get text style for labels
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] Text::TextStyle const &labelStyle(int nestingLevel) const noexcept {
        return subTheme(themeMode).labelStyle(nestingLevel);
    }

    [[nodiscard]] Text::TextStyle const &warningLabelStyle(int nestingLevel) const noexcept {
        return subTheme(themeMode).warningLabelStyle;
    }

    [[nodiscard]] Text::TextStyle const &errorLabelStyle(int nestingLevel) const noexcept {
        return subTheme(themeMode).errorLabelStyle;
    }

    [[nodiscard]] Text::TextStyle const &helpLabelStyle(int nestingLevel) const noexcept {
        return subTheme(themeMode).helpLabelStyle;
    }

    [[nodiscard]] Text::TextStyle const &linkLabelStyle(int nestingLevel) const noexcept {
        return subTheme(themeMode).linkLabelStyle;
    }

    //TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, labelColor, 0.0, TextDecoration::None);
};

inline Theme theme;

}
