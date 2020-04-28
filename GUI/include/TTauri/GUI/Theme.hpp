// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Text/TextStyle.hpp"
#include "TTauri/GUI/ThemeMode.hpp"
#include <array>

namespace TTauri::GUI {

class Theme {
private:
    std::vector<vec> fillShades;
    std::vector<vec> borderShades;
    std::vector<vec> grayShades;

public:
    /** Distance between widgets and between widgets and the border of the container.
     */
    static constexpr float margin = 6.0f;

    /** The line-width of a border.
     */
    static constexpr float borderWidth = 1.0f;

    /** The rounding radius of boxes with rounded corners.
     */
    static constexpr float roundingRadius = 3.0f;

    /** The height of smaller widget like labels, toggles, checkboxes and radio buttons.
     */
    static constexpr float smallHeight = 16.0f;

    /** The width of smaller widget like labels, toggles, checkboxes and radio buttons.
     * Small widgets which include labels should be right aligned to the `smallWidth`
     * with a `margin` between the widget and the included label.
     */
    static constexpr float smallWidth = smallHeight * 2.0f;

    /** The height of the larger widgets like buttons, text-input and drop-down-lists.
     */
    static constexpr float height = 22.0f;

    /** The width of the larger widgets and smaller widgets with included labels.
     */
    static constexpr float width = 150.0f;

    std::string name;
    ThemeMode mode;

    // Themed bright colors.
    vec blue;
    vec green;
    vec indigo;
    vec orange;
    vec pink;
    vec purple;
    vec red;
    vec teal;
    vec yellow;

    // Semantic colors
    vec foregroundColor;
    vec accentColor;
    vec textSelectColor;
    vec cursorColor;
    vec incompleteGlyphColor;

    Text::TextStyle labelStyle;
    Text::TextStyle smallLabelStyle;
    Text::TextStyle warningLabelStyle;
    Text::TextStyle errorLabelStyle;
    Text::TextStyle helpLabelStyle;
    Text::TextStyle placeholderLabelStyle;
    Text::TextStyle linkLabelStyle;

    Theme() noexcept = delete;
    Theme(Theme const &) noexcept = delete;
    Theme(Theme &&) noexcept = delete;
    Theme &operator=(Theme const &) noexcept = delete;
    Theme &operator=(Theme &&) noexcept = delete;

    /** Open and parse a theme file.
     */
    Theme(URL const &url);

    /** Get fill color of elements of widgets and child widgets.
    * @param nestingLevel The nesting level.
    */
    [[nodiscard]] vec fillColor(ssize_t nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        ttauri_assume(ssize(fillShades) > 0);
        return fillShades[nestingLevel % ssize(fillShades)];
    }

    /** Get border color of elements of widgets and child widgets.
    * @param nestingLevel The nesting level.
    */
    [[nodiscard]] vec borderColor(ssize_t nestingLevel) const noexcept {
        ttauri_assume(nestingLevel >= 0);
        ttauri_assume(ssize(borderShades) > 0);
        return borderShades[nestingLevel % ssize(borderShades)];
    }


    /** Get grey scale color
    * This color is reversed between light and dark themes.
    * @param level Gray level: 0 is background, positive values increase in foregroundness.
    *              -1 is foreground, more negative values go toward background.
    */
    [[nodiscard]] vec gray(ssize_t level) const noexcept {
        if (level < 0) {
            level = ssize(grayShades) + level;
        }

        level = std::clamp(level, ssize_t{0}, ssize(grayShades) - 1);
        return grayShades[level];
    }

private:
    [[nodiscard]] float parseFloat(datum const &data, char const *name);
    [[nodiscard]] bool parseBool(datum const &data, char const *name);
    [[nodiscard]] std::string parseString(datum const &data, char const *name);
    [[nodiscard]] vec parseColorValue(datum const &data);
    [[nodiscard]] std::vector<vec> parseColorList(datum const &data, char const *name);
    [[nodiscard]] vec parseColor(datum const &data, char const *name);
    [[nodiscard]] Text::TextStyle parseTextStyleValue(datum const &data);
    [[nodiscard]] Text::FontWeight parseFontWeight(datum const &data, char const *name);
    [[nodiscard]] Text::TextStyle parseTextStyle(datum const &data, char const *name);
    void parse(datum const &data);

    [[nodiscard]] friend std::string to_string(Theme const &rhs) noexcept {
        return fmt::format("{}:{}", rhs.name, rhs.mode);
    }

    friend std::ostream &operator<<(std::ostream &lhs, Theme const &rhs) {
        return lhs << to_string(rhs);
    }
};

inline Theme *theme;

}
