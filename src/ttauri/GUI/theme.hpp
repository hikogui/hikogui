// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "../required.hpp"
#include "../text/text_style.hpp"
#include "../URL.hpp"
#include "../datum.hpp"
#include "../color/color.hpp"
#include <array>

namespace tt {

class theme {
public:
    static inline theme *global;

    static constexpr OperatingSystem operatingSystem = OperatingSystem::Windows;

    float toolbarHeight =
        (operatingSystem == OperatingSystem::Windows) ? 30.0f : 20.0f;

    /** The width of a close, minimize, maximize, system menu button.
     */
    float toolbarDecorationButtonWidth =
        (operatingSystem == OperatingSystem::Windows) ? 30.0f : 20.0f;

    /** Distance between widgets and between widgets and the border of the container.
     */
    float margin = 6.0f;

    extent2 margin2D = extent2{margin, margin};
    extent2 margin2Dx2 = extent2{margin * 2.0f, margin * 2.0f};

    float scroll_bar_thickness = margin * 2.0f;

    /** The line-width of a border.
     */
    float borderWidth = 1.0f;

    /** The rounding radius of boxes with rounded corners.
     */
    float roundingRadius = 5.0f;

    /** The size of small square widgets.
     */
    float smallSize = 15.0f;

    /** The height of the larger widgets like buttons, text-input and drop-down-lists.
     */
    float height = 22.0f;

    /** The width of the larger widgets and smaller widgets with included labels.
     */
    float width = 50.0f;

    /** Max width of labels in widgets.
     */
    float maxLabelWidth = 300.0f;

    /** Size of icons that represents the size of label's text.
     */
    float small_icon_size = 10.0f;

    /** Size of icons extending from the ascender to descender of a label's text.
     */
    float icon_size = 20.0f;

    /** Size of icons representing the length of am average word of a label's text.
     */
    float large_icon_size = 30.0f;


    std::string name;
    theme_mode mode;

    // Themed bright colors.
    color blue;
    color green;
    color indigo;
    color orange;
    color pink;
    color purple;
    color red;
    color teal;
    color yellow;

    // Semantic colors
    color foregroundColor;
    color accentColor;
    color textSelectColor;
    color cursorColor;
    color incompleteGlyphColor;

    text_style labelStyle;
    text_style smallLabelStyle;
    text_style warningLabelStyle;
    text_style errorLabelStyle;
    text_style helpLabelStyle;
    text_style placeholderLabelStyle;
    text_style linkLabelStyle;

    theme() noexcept = delete;
    theme(theme const &) noexcept = delete;
    theme(theme &&) noexcept = delete;
    theme &operator=(theme const &) noexcept = delete;
    theme &operator=(theme &&) noexcept = delete;

    /** Open and parse a theme file.
     */
    theme(URL const &url);

    /** Get fill color of elements of widgets and child widgets.
    * @param nestingLevel The nesting level.
    */
    [[nodiscard]] color fillColor(ssize_t nesting_level) const noexcept
    {
        nesting_level = std::max(ssize_t{0}, nesting_level);
        tt_axiom(std::ssize(fillShades) > 0);
        return fillShades[nesting_level % std::ssize(fillShades)];
    }

    /** Get border color of elements of widgets and child widgets.
    * @param nestingLevel The nesting level.
    */
    [[nodiscard]] color borderColor(ssize_t nesting_level) const noexcept
    {
        nesting_level = std::max(ssize_t{0}, nesting_level);
        tt_axiom(std::ssize(borderShades) > 0);
        return borderShades[nesting_level % std::ssize(borderShades)];
    }


    /** Get grey scale color
    * This color is reversed between light and dark themes.
    * @param level Gray level: 0 is background, positive values increase in foregroundness.
    *              -1 is foreground, more negative values go toward background.
    */
    [[nodiscard]] color gray(ssize_t level) const noexcept
    {
        if (level < 0) {
            level = std::ssize(grayShades) + level;
        }

        level = std::clamp(level, ssize_t{0}, std::ssize(grayShades) - 1);
        return grayShades[level];
    }

private:
    std::vector<color> fillShades;
    std::vector<color> borderShades;
    std::vector<color> grayShades;

    [[nodiscard]] float parseFloat(datum const &data, char const *name);
    [[nodiscard]] bool parseBool(datum const &data, char const *name);
    [[nodiscard]] std::string parseString(datum const &data, char const *name);
    [[nodiscard]] color parseColorValue(datum const &data);
    [[nodiscard]] std::vector<color> parseColorList(datum const &data, char const *name);
    [[nodiscard]] color parseColor(datum const &data, char const *name);
    [[nodiscard]] text_style parsetext_styleValue(datum const &data);
    [[nodiscard]] font_weight parsefont_weight(datum const &data, char const *name);
    [[nodiscard]] text_style parsetext_style(datum const &data, char const *name);
    void parse(datum const &data);

    [[nodiscard]] friend std::string to_string(theme const &rhs) noexcept {
        return fmt::format("{}:{}", rhs.name, rhs.mode);
    }

    friend std::ostream &operator<<(std::ostream &lhs, theme const &rhs) {
        return lhs << to_string(rhs);
    }
};

}
