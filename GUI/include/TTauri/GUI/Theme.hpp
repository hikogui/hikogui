// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Text/FontDescription.hpp"
#include "TTauri/Text/TextStyle.hpp"
#include <array>

namespace TTauri::GUI {

enum class SubThemeType {
    Light,
    Dark,
    LightAccessable,
    DarkAccessable
};

class SubTheme {
    std::vector<vec> colors;
    std::vector<vec> fillColors;
    std::vector<vec> borderColors;

    std::array<vec,11> grayColors;

    std::vector<TextStyle> labelStyles;

public:
    TextStyle warningLabelStyle;
    TextStyle errorLabelStyle;
    TextStyle helpLabelStyle;
    TextStyle linkLabelStyle;

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
        return colors[nestingLevel % ssize(colors)];
    }

    /** Get fill color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec fillColor(int nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        return fillColors[nestingLevel % ssize(fillColors)];
    }

    /** Get border color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec borderColor(int nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        return borderColors[nestingLevel % ssize(borderColors)];
    }

    /** Get grey scale color
     * This color is reversed between light and dark themes.
     * @param level Color 5 foreground, 0 mid-gray, -5 background
     */
    [[nodiscard]] vec grayColor(int level) const noexcept {
        constexpr int maxLevel = static_cast<int>(grayColors.size()) / 2;
        constexpr int minLevel = -maxLevel;
        int level_ = std::clamp(minLevel, level, maxLevel) + maxLevel;
        return borderColors[level_];
    }


    /** Get border color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] TextStyle const &labelStyle(int nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        return labelStyle[nestingLevel % ssize(labelStyles)];
    }
};

class Theme {
private:
    std::array<SubTheme,4> subThemes;

    SubTheme const &subTheme(SubThemeType type) const noexcept {
        return subThemes[static_cast<int>(type)];
    }

public:

    /** Get color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec color(int nestingLevel) const noexcept {
        return subTheme(selectedSubTheme).color(nestingLevel);
    }

    /** Get fill color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec fillColor(int nestingLevel) const noexcept {
        return subTheme(selectedSubTheme).fillColors(nestingLevel);
    }

    /** Get border color
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] vec borderColor(int nestingLevel) const noexcept {
        return subTheme(selectedSubTheme).borderColors(nestingLevel);
    }

    /** Get grey scale color
     * This color is reversed between light and dark themes.
     * @param level Color 5 foreground, 0 mid-gray, -5 background
     */
    [[nodiscard]] vec grayColor(int level) const noexcept {
        return subTheme(selectedSubTheme).grayColor(level);
    }


    [[nodiscard]] vec blueColor() const noexcept {
        return subTheme(selectedSubTheme).blueColor;
    }

    [[nodiscard]] vec greenColor() const noexcept {
        return subTheme(selectedSubTheme).greenColor;
    }

    [[nodiscard]] vec indigoColor() const noexcept {
        return subTheme(selectedSubTheme).indigoColor;
    }

    [[nodiscard]] vec orangeColor() const noexcept {
        return subTheme(selectedSubTheme).orangeColor;
    }

    [[nodiscard]] vec pinkColor() const noexcept {
        return subTheme(selectedSubTheme).pinkColor;
    }

    [[nodiscard]] vec purpleColor() const noexcept {
        return subTheme(selectedSubTheme).purpleColor;
    }

    [[nodiscard]] vec redColor() const noexcept {
        return subTheme(selectedSubTheme).redColor;
    }

    [[nodiscard]] vec tealColor() const noexcept {
        return subTheme(selectedSubTheme).tealColor;
    }

    [[nodiscard]] vec yellowColor() const noexcept {
        return subTheme(selectedSubTheme).yellowColor;
    }



    /** Get text style for labels
     * @param nestingLevel The nesting level.
     */
    [[nodiscard]] TextStyle &labelStyle(int nestingLevel) const noexcept {
        return subTheme(selectedSubTheme).labelStyles(nestingLevel);
    }

    [[nodiscard]] TextStyle &warningLabelStyle(int nestingLevel) const noexcept {
        return subTheme(selectedSubTheme).warningLabelStyle;
    }

    [[nodiscard]] TextStyle &errorLabelStyle(int nestingLevel) const noexcept {
        return subTheme(selectedSubTheme).errorLabelStyle;
    }

    [[nodiscard]] TextStyle &helpLabelStyle(int nestingLevel) const noexcept {
        return subTheme(selectedSubTheme).helpLabelStyle;
    }

    [[nodiscard]] TextStyle &linkLabelStyle(int nestingLevel) const noexcept {
        return subTheme(selectedSubTheme).linkLabelStyle;
    }

    //TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, labelColor, 0.0, TextDecoration::None);
};

}
