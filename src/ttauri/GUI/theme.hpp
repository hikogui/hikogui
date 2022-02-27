// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "theme_color.hpp"
#include "theme_text_style.hpp"
#include "../required.hpp"
#include "../text/text_style.hpp"
#include "../URL.hpp"
#include "../datum.hpp"
#include "../color/color.hpp"
#include "../geometry/extent.hpp"
#include "../subsystem.hpp"
#include <array>

namespace tt::inline v1 {
class font_book;

class theme {
public:
    operating_system operating_system = operating_system::windows;

    /** The DPI of the size values.
     */
    float dpi = 72;

    /** The scale factor used to convert pt to physical pixel size.
     */
    float scale = 1.0f;

    /** Distance between widgets and between widgets and the border of the container.
     */
    float margin = 4.5f;

    /** The line-width of a border.
     */
    float border_width = 1.0f;

    /** The rounding radius of boxes with rounded corners.
     */
    float rounding_radius = 4.0f;

    /** The size of small square widgets.
     */
    float size = 11.0f;

    /** The size of large widgets. Such as the minimum scroll bar size.
     */
    float large_size = 18.5f;

    /** Size of icons inside a widget.
     */
    float icon_size = 7.5f;

    /** Size of icons representing the length of am average word of a label's text.
     */
    float large_icon_size = 22.5f;

    /** Size of icons being inline with a label's text.
     */
    float label_icon_size = 15.0f;

    std::string name;
    theme_mode mode = theme_mode::light;

    theme() noexcept = default;
    theme(theme const &) noexcept = default;
    theme(theme &&) noexcept = default;
    theme &operator=(theme const &) noexcept = default;
    theme &operator=(theme &&) noexcept = default;

    /** Open and parse a theme file.
     */
    theme(tt::font_book const &font_book, URL const &url);

    /** Create a transformed copy of the theme.
    * 
    * This function is used by the window, to make a specific version of
    * the theme scaled to the dpi of the window.
    * 
    * It can also create a different version when the window becomes active/inactive
    * mostly this will desaturate the colors in the theme.
    * 
    * @param dpi The dpi of the window.
    */
    [[nodiscard]] theme transform(float dpi) const noexcept;

    [[nodiscard]] tt::color color(theme_color theme_color, ssize_t nesting_level = 0) const noexcept;
    [[nodiscard]] tt::text_style const &text_style(theme_text_style theme_color) const noexcept;

private:
    std::array<std::vector<tt::color>, num_theme_colors> _colors;
    std::array<tt::text_style, num_theme_text_styles> _text_styles;

    [[nodiscard]] float parse_float(datum const &data, char const *object_name);
    [[nodiscard]] bool parse_bool(datum const &data, char const *object_name);
    [[nodiscard]] std::string parse_string(datum const &data, char const *object_name);
    [[nodiscard]] tt::color parse_color_value(datum const &data);
    [[nodiscard]] tt::color parse_color(datum const &data, char const *object_name);
    [[nodiscard]] std::vector<tt::color> parse_color_list(datum const &data, char const *object_name);
    [[nodiscard]] tt::text_style parse_text_style_value(tt::font_book const &font_book, datum const &data);
    [[nodiscard]] font_weight parse_font_weight(datum const &data, char const *object_name);
    [[nodiscard]] tt::text_style parse_text_style(tt::font_book const &font_book, datum const &data, char const *object_name);
    void parse(tt::font_book const &font_book, datum const &data);

    [[nodiscard]] friend std::string to_string(theme const &rhs) noexcept
    {
        return std::format("{}:{}", rhs.name, rhs.mode);
    }

    friend std::ostream &operator<<(std::ostream &lhs, theme const &rhs)
    {
        return lhs << to_string(rhs);
    }
};

} // namespace tt::inline v1
