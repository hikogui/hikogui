// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "../text/semantic_text_style.hpp"
#include "../text/text_style.hpp"
#include "../utility/module.hpp"
#include "../datum.hpp"
#include "../color/module.hpp"
#include "../geometry/module.hpp"
#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace hi::inline v1 {
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

    std::string name;
    theme_mode mode = theme_mode::light;

    theme() noexcept = default;
    theme(theme const&) noexcept = default;
    theme(theme&&) noexcept = default;
    theme& operator=(theme const&) noexcept = default;
    theme& operator=(theme&&) noexcept = default;

    /** Open and parse a theme file.
     */
    theme(hi::font_book const& font_book, std::filesystem::path const& url);

    /** Distance between widgets and between widgets and the border of the container.
     */
    template<typename T = hi::marginsi>
    [[nodiscard]] constexpr T margin() const noexcept
    {
        if constexpr (std::is_same_v<T, hi::marginsi>) {
            return hi::marginsi{_margin};
        } else {
            return narrow_cast<T>(_margin);
        }
    }

    /** The line-width of a border.
     */
    template<arithmetic T = int>
    [[nodiscard]] constexpr T border_width() const noexcept
    {
        return narrow_cast<T>(_border_width);
    }

    /** The rounding radius of boxes with rounded corners.
     */
    template<typename T = hi::corner_radii>
    [[nodiscard]] constexpr T rounding_radius() const noexcept
    {
        if constexpr (std::is_same_v<T, hi::corner_radii>) {
            return T{narrow_cast<float>(_rounding_radius)};
        } else {
            return narrow_cast<T>(_rounding_radius);
        }
    }

    /** The size of small square widgets.
     */
    template<arithmetic T = int>
    [[nodiscard]] constexpr T size() const noexcept
    {
        return narrow_cast<T>(_size);
    }

    /** The size of large widgets. Such as the minimum scroll bar size.
     */
    template<arithmetic T = int>
    [[nodiscard]] constexpr T large_size() const noexcept
    {
        return narrow_cast<T>(_large_size);
    }

    /** Size of icons inside a widget.
     */
    template<arithmetic T = int>
    [[nodiscard]] constexpr T icon_size() const noexcept
    {
        return narrow_cast<T>(_icon_size);
    }

    /** Size of icons representing the length of am average word of a label's text.
     */
    template<arithmetic T = int>
    [[nodiscard]] constexpr T large_icon_size() const noexcept
    {
        return narrow_cast<T>(_large_icon_size);
    }

    /** Size of icons being inline with a label's text.
     */
    template<arithmetic T = int>
    [[nodiscard]] constexpr T label_icon_size() const noexcept
    {
        return narrow_cast<T>(_label_icon_size);
    }

    /** The amount the base-line needs to be moved downwards when a label is aligned to top.
     */
    template<arithmetic T = int>
    [[nodiscard]] constexpr T baseline_adjustment() const noexcept
    {
        return narrow_cast<T>(_baseline_adjustment);
    }

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

    [[nodiscard]] hi::color color(hi::semantic_color original_color, ssize_t nesting_level = 0) const noexcept;
    [[nodiscard]] hi::color color(hi::color original_color, ssize_t nesting_level = 0) const noexcept;
    [[nodiscard]] hi::text_style text_style(semantic_text_style theme_color) const noexcept;
    [[nodiscard]] hi::text_style text_style(hi::text_style original_style) const noexcept;

private:
    /** Distance between widgets and between widgets and the border of the container.
     */
    int _margin = 5;

    /** The line-width of a border.
     */
    int _border_width = 1;

    /** The rounding radius of boxes with rounded corners.
     */
    int _rounding_radius = 4;

    /** The size of small square widgets.
     */
    int _size = 11;

    /** The size of large widgets. Such as the minimum scroll bar size.
     */
    int _large_size = 19;

    /** Size of icons inside a widget.
     */
    int _icon_size = 8;

    /** Size of icons representing the length of am average word of a label's text.
     */
    int _large_icon_size = 23;

    /** Size of icons being inline with a label's text.
     */
    int _label_icon_size = 15;

    /** The amount the base-line needs to be moved downwards when a label is aligned to top.
     */
    int _baseline_adjustment = 9;

    std::array<std::vector<hi::color>, semantic_color_metadata.size()> _colors;
    std::array<hi::text_style, semantic_text_style_metadata.size()> _text_styles;

    [[nodiscard]] float parse_float(datum const& data, char const *object_name);
    [[nodiscard]] long long parse_long_long(datum const& data, char const *object_name);
    [[nodiscard]] int parse_int(datum const& data, char const *object_name);
    [[nodiscard]] bool parse_bool(datum const& data, char const *object_name);
    [[nodiscard]] std::string parse_string(datum const& data, char const *object_name);
    [[nodiscard]] hi::color parse_color_value(datum const& data);
    [[nodiscard]] hi::color parse_color(datum const& data, char const *object_name);
    [[nodiscard]] std::vector<hi::color> parse_color_list(datum const& data, char const *object_name);
    [[nodiscard]] hi::text_style parse_text_style_value(hi::font_book const& font_book, datum const& data);
    [[nodiscard]] font_weight parse_font_weight(datum const& data, char const *object_name);
    [[nodiscard]] hi::text_style parse_text_style(hi::font_book const& font_book, datum const& data, char const *object_name);
    void parse(hi::font_book const& font_book, datum const& data);

    [[nodiscard]] friend std::string to_string(theme const& rhs) noexcept
    {
        return std::format("{}:{}", rhs.name, rhs.mode);
    }

    friend std::ostream& operator<<(std::ostream& lhs, theme const& rhs)
    {
        return lhs << to_string(rhs);
    }
};

} // namespace hi::inline v1
