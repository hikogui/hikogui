// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ThemeMode.hpp"
#include "../required.hpp"
#include "../mat.hpp"
#include "../text/TextStyle.hpp"
#include <array>

namespace tt {

class Theme {
private:
    std::vector<vec> fillShades;
    std::vector<vec> borderShades;
    std::vector<vec> grayShades;

public:
    static constexpr OperatingSystem operatingSystem = OperatingSystem::Windows;

    static inline float toolbarHeight =
        (operatingSystem == OperatingSystem::Windows) ? 30.0f : 20.0f;

    /** The width of a close, minimize, maximize, system menu button.
     */
    static inline float toolbarDecorationButtonWidth =
        (operatingSystem == OperatingSystem::Windows) ? 30.0f : 20.0f;

    /** Distance between widgets and between widgets and the border of the container.
     */
    static constexpr float margin = 6.0f;

    static inline const vec margin2D = vec{margin, margin};

    /** The line-width of a border.
     */
    static constexpr float borderWidth = 1.0f;

    /** The rounding radius of boxes with rounded corners.
     */
    static constexpr float roundingRadius = 5.0f;

    /** The height of smaller widget like labels, toggles, checkboxes and radio buttons.
     */
    static constexpr float smallHeight = 15.0f;

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
    static constexpr float width = 50.0f;

    /** Max width of labels in widgets.
     */
    static constexpr float maxLabelWidth = 300.0f;

    /** Size of icons in buttons, based on the original 1EM.
     */
    static constexpr float iconSize = 10.0;

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

    TextStyle labelStyle;
    TextStyle smallLabelStyle;
    TextStyle warningLabelStyle;
    TextStyle errorLabelStyle;
    TextStyle helpLabelStyle;
    TextStyle placeholderLabelStyle;
    TextStyle linkLabelStyle;

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
        tt_assume(nestingLevel >= 0);
        tt_assume(ssize(fillShades) > 0);
        return fillShades[nestingLevel % ssize(fillShades)];
    }

    /** Get border color of elements of widgets and child widgets.
    * @param nestingLevel The nesting level.
    */
    [[nodiscard]] vec borderColor(ssize_t nestingLevel) const noexcept {
        tt_assume(nestingLevel >= 0);
        tt_assume(ssize(borderShades) > 0);
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
    [[nodiscard]] TextStyle parseTextStyleValue(datum const &data);
    [[nodiscard]] FontWeight parseFontWeight(datum const &data, char const *name);
    [[nodiscard]] TextStyle parseTextStyle(datum const &data, char const *name);
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
