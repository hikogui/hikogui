// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "theme_color.hpp"
#include "../required.hpp"
#include "../text/text_style.hpp"
#include "../URL.hpp"
#include "../datum.hpp"
#include "../color/color.hpp"
#include "../geometry/extent.hpp"
#include <array>

namespace tt {

class theme {
public:
    operating_system operating_system = operating_system::windows;

    float toolbar_height = (operating_system == operating_system::windows) ? 30.0f : 20.0f;

    /** The width of a close, minimize, maximize, system menu button.
     */
    float toolbar_decoration_button_width = (operating_system == operating_system::windows) ? 30.0f : 20.0f;

    /** Distance between widgets and between widgets and the border of the container.
     */
    float margin = 6.0f;

    /** The line-width of a border.
     */
    float border_width = 1.0f;

    /** The rounding radius of boxes with rounded corners.
     */
    float rounding_radius = 5.0f;

    /** The size of small square widgets.
     */
    float size = 15.0f;

    /** The size of large widgets. Such as the minimum scroll bar size.
     */
    float large_size = 25.0f;

    /** Size of icons inside a widget.
     */
    float icon_size = 10.0f;

    /** Size of icons representing the length of am average word of a label's text.
     */
    float large_icon_size = 30.0f;

    /** Size of icons being inline with a label's text.
     */
    float label_icon_size = 20.0f;


    std::string name;
    theme_mode mode;

    

    text_style label_style;
    text_style small_label_style;
    text_style warning_label_style;
    text_style error_label_style;
    text_style help_label_style;
    text_style placeholder_label_style;
    text_style link_label_style;

    theme() noexcept = delete;
    theme(theme const &) noexcept = delete;
    theme(theme &&) noexcept = delete;
    theme &operator=(theme const &) noexcept = delete;
    theme &operator=(theme &&) noexcept = delete;

    /** Open and parse a theme file.
     */
    theme(URL const &url);

    [[nodiscard]] tt::color color(theme_color theme_color, ssize_t nesting_level=0) const noexcept;

    static void set_global(theme *theme) noexcept
    {
        _global = theme;
    }

    [[nodiscard]] static theme &global() noexcept
    {
        tt_axiom(_global);
        return *_global;
    }

    [[nodiscard]] static tt::color global(theme_color color, ssize_t nesting_level=0) noexcept
    {
        return global().color(color, nesting_level);
    }

private:
    static inline theme *_global = nullptr;

    std::array<std::vector<tt::color>, num_theme_colors> _colors;

    [[nodiscard]] float parse_float(datum const &data, char const *object_name);
    [[nodiscard]] bool parse_bool(datum const &data, char const *object_name);
    [[nodiscard]] std::string parse_string(datum const &data, char const *object_name);
    [[nodiscard]] tt::color parse_color_value(datum const &data);
    [[nodiscard]] tt::color parse_color(datum const &data, char const *object_name);
    [[nodiscard]] std::vector<tt::color> parse_color_list(datum const &data, char const *object_name);
    [[nodiscard]] text_style parse_text_style_value(datum const &data);
    [[nodiscard]] font_weight parse_font_weight(datum const &data, char const *object_name);
    [[nodiscard]] text_style parse_text_style(datum const &data, char const *object_name);
    void parse(datum const &data);

    [[nodiscard]] friend std::string to_string(theme const &rhs) noexcept {
        return std::format("{}:{}", rhs.name, rhs.mode);
    }

    friend std::ostream &operator<<(std::ostream &lhs, theme const &rhs) {
        return lhs << to_string(rhs);
    }
};

}
