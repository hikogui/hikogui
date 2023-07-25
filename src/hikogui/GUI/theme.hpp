// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../settings/module.hpp"
#include "../text/module.hpp"
#include "../utility/module.hpp"
#include "../color/module.hpp"
#include "../geometry/module.hpp"
#include "../codec/module.hpp"
#include "../macros.hpp"
#include <array>
#include <filesystem>
#include <string>
#include <vector>



namespace hi::inline v1 {
class font_book;

class theme {
public:
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
    template<typename T = hi::margins>
    [[nodiscard]] constexpr T margin() const noexcept
    {
        if constexpr (std::is_same_v<T, hi::margins>) {
            return hi::margins{_margin};
        } else if constexpr (std::is_same_v<T, float>) {
            return _margin;
        } else {
            hi_static_not_implemented();
        }
    }

    /** The line-width of a border.
     */
    [[nodiscard]] constexpr float border_width() const noexcept
    {
        return _border_width;
    }

    /** The rounding radius of boxes with rounded corners.
     */
    template<typename T = hi::corner_radii>
    [[nodiscard]] constexpr T rounding_radius() const noexcept
    {
        if constexpr (std::is_same_v<T, hi::corner_radii>) {
            return T{_rounding_radius};
        } else if constexpr (std::is_same_v<T, float>) {
            return _rounding_radius;
        } else {
            hi_static_not_implemented();
        }
    }

    /** The size of small square widgets.
     */
    [[nodiscard]] constexpr float size() const noexcept
    {
        return _size;
    }

    /** The size of large widgets. Such as the minimum scroll bar size.
     */
    [[nodiscard]] constexpr float large_size() const noexcept
    {
        return _large_size;
    }

    /** Size of icons inside a widget.
     */
    [[nodiscard]] constexpr float icon_size() const noexcept
    {
        return _icon_size;
    }

    /** Size of icons representing the length of am average word of a label's text.
     */
    [[nodiscard]] constexpr float large_icon_size() const noexcept
    {
        return _large_icon_size;
    }

    /** Size of icons being inline with a label's text.
     */
    [[nodiscard]] constexpr float label_icon_size() const noexcept
    {
        return _label_icon_size;
    }

    /** The amount the base-line needs to be moved downwards when a label is aligned to top.
     */
    [[nodiscard]] constexpr float baseline_adjustment() const noexcept
    {
        return _baseline_adjustment;
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
    float _margin = 5.0f;

    /** The line-width of a border.
     */
    float _border_width = 1.0f;

    /** The rounding radius of boxes with rounded corners.
     */
    float _rounding_radius = 4.0f;

    /** The size of small square widgets.
     */
    float _size = 11.0f;

    /** The size of large widgets. Such as the minimum scroll bar size.
     */
    float _large_size = 19.0f;

    /** Size of icons inside a widget.
     */
    float _icon_size = 8.0f;

    /** Size of icons representing the length of am average word of a label's text.
     */
    float _large_icon_size = 23.0f;

    /** Size of icons being inline with a label's text.
     */
    float _label_icon_size = 15.0f;

    /** The amount the base-line needs to be moved downwards when a label is aligned to top.
     */
    float _baseline_adjustment = 9.0f;

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
