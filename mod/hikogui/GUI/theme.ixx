// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

export module hikogui_GUI : theme;
import hikogui_codec;
import hikogui_color;
import hikogui_geometry;
import hikogui_settings;
import hikogui_text;
import hikogui_utility;

export namespace hi::inline v1 {

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
    theme(std::filesystem::path const& path)
    {
        try {
            hi_log_info("Parsing theme at {}", path.string());
            hilet data = parse_JSON(path);
            parse(data);
        } catch (std::exception const& e) {
            throw io_error(std::format("{}: Could not load theme.\n{}", path.string(), e.what()));
        }
    }

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

    /** Size of icons being with a label's text.
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
    [[nodiscard]] theme transform(float new_dpi) const noexcept
    {
        auto r = *this;

        hi_assert(new_dpi != 0.0f);
        hi_assert(dpi != 0.0f);
        hi_assert(scale != 0.0f);

        auto delta_scale = new_dpi / dpi;
        r.dpi = new_dpi;
        r.scale = delta_scale * scale;

        // Scale each size, and round so that everything will stay aligned on pixel boundaries.
        r._margin = std::round(delta_scale * _margin);
        r._border_width = std::round(delta_scale * _border_width);
        r._rounding_radius = std::round(delta_scale * _rounding_radius);
        r._size = std::round(delta_scale * _size);
        r._large_size = std::round(delta_scale * _large_size);
        r._icon_size = std::round(delta_scale * _icon_size);
        r._large_icon_size = std::round(delta_scale * _large_icon_size);
        r._label_icon_size = std::round(delta_scale * _label_icon_size);
        // Cap height is not rounded, since the text-shaper will align the text to sub-pixel boundaries.
        r._baseline_adjustment = std::round(delta_scale * _baseline_adjustment);

        return r;
    }

    [[nodiscard]] hi::color color(hi::semantic_color original_color, ssize_t nesting_level = 0) const noexcept
    {
        hilet& shades = _colors[std::to_underlying(original_color)];
        hi_assert(not shades.empty());

        nesting_level = std::max(ssize_t{0}, nesting_level);
        return shades[nesting_level % ssize(shades)];
    }

    [[nodiscard]] hi::color color(hi::color original_color, ssize_t nesting_level = 0) const noexcept
    {
        if (original_color.is_semantic()) {
            return color(static_cast<semantic_color>(original_color), nesting_level);
        } else {
            return original_color;
        }
    }

    [[nodiscard]] hi::text_style text_style(semantic_text_style theme_color) const noexcept
    {
        return _text_styles[std::to_underlying(theme_color)];
    }

    [[nodiscard]] hi::text_style text_style(hi::text_style original_style) const noexcept
    {
        if (original_style.is_semantic()) {
            return text_style(static_cast<semantic_text_style>(original_style));
        } else {
            return original_style;
        }
    }

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

    /** Size of icons being with a label's text.
     */
    float _label_icon_size = 15.0f;

    /** The amount the base-line needs to be moved downwards when a label is aligned to top.
     */
    float _baseline_adjustment = 9.0f;

    std::array<std::vector<hi::color>, semantic_color_metadata.size()> _colors;
    std::array<hi::text_style, semantic_text_style_metadata.size()> _text_styles;

    [[nodiscard]] float parse_float(datum const& data, char const *object_name)
    {
        if (!data.contains(object_name)) {
            throw parse_error(std::format("Missing '{}'", object_name));
        }

        hilet object = data[object_name];
        if (auto f = get_if<double>(object)) {
            return static_cast<float>(*f);
        } else if (auto ll = get_if<long long>(object)) {
            return static_cast<float>(*ll);
        } else {
            throw parse_error(
                std::format("'{}' attribute must be a floating point number, got {}.", object_name, object.type_name()));
        }
    }

    [[nodiscard]] long long parse_long_long(datum const& data, char const *object_name)
    {
        if (!data.contains(object_name)) {
            throw parse_error(std::format("Missing '{}'", object_name));
        }

        hilet object = data[object_name];
        if (auto f = get_if<long long>(object)) {
            return static_cast<long long>(*f);
        } else {
            throw parse_error(std::format("'{}' attribute must be a integer, got {}.", object_name, object.type_name()));
        }
    }

    [[nodiscard]] int parse_int(datum const& data, char const *object_name)
    {
        hilet value = parse_long_long(data, object_name);
        if (value > std::numeric_limits<int>::max() or value < std::numeric_limits<int>::min()) {
            throw parse_error(std::format("'{}' attribute is out of range, got {}.", object_name, value));
        }
        return narrow_cast<int>(value);
    }

    [[nodiscard]] bool parse_bool(datum const& data, char const *object_name)
    {
        if (!data.contains(object_name)) {
            throw parse_error(std::format("Missing '{}'", object_name));
        }

        hilet object = data[object_name];
        if (!holds_alternative<bool>(object)) {
            throw parse_error(std::format("'{}' attribute must be a boolean, got {}.", object_name, object.type_name()));
        }

        return to_bool(object);
    }

    [[nodiscard]] std::string parse_string(datum const& data, char const *object_name)
    {
        // Extract name
        if (!data.contains(object_name)) {
            throw parse_error(std::format("Missing '{}'", object_name));
        }
        hilet object = data[object_name];
        if (!holds_alternative<std::string>(object)) {
            throw parse_error(std::format("'{}' attribute must be a string, got {}.", object_name, object.type_name()));
        }
        return static_cast<std::string>(object);
    }

    [[nodiscard]] hi::color parse_color_value(datum const& data)
    {
        if (holds_alternative<datum::vector_type>(data)) {
            if (data.size() != 3 && data.size() != 4) {
                throw parse_error(std::format("Expect 3 or 4 values for a color, got {}.", data));
            }
            hilet r = data[0];
            hilet g = data[1];
            hilet b = data[2];
            hilet a = data.size() == 4 ? data[3] : (holds_alternative<long long>(r) ? datum{255} : datum{1.0});

            if (holds_alternative<long long>(r) and holds_alternative<long long>(g) and holds_alternative<long long>(b) and
                holds_alternative<long long>(a)) {
                hilet r_ = get<long long>(r);
                hilet g_ = get<long long>(g);
                hilet b_ = get<long long>(b);
                hilet a_ = get<long long>(a);

                hi_check(r_ >= 0 and r_ <= 255, "integer red-color value not within 0 and 255");
                hi_check(g_ >= 0 and g_ <= 255, "integer green-color value not within 0 and 255");
                hi_check(b_ >= 0 and b_ <= 255, "integer blue-color value not within 0 and 255");
                hi_check(a_ >= 0 and a_ <= 255, "integer alpha-color value not within 0 and 255");

                return color_from_sRGB(
                    static_cast<uint8_t>(r_), static_cast<uint8_t>(g_), static_cast<uint8_t>(b_), static_cast<uint8_t>(a_));

            } else if (
                holds_alternative<double>(r) and holds_alternative<double>(g) and holds_alternative<double>(b) and
                holds_alternative<double>(a)) {
                hilet r_ = static_cast<float>(get<double>(r));
                hilet g_ = static_cast<float>(get<double>(g));
                hilet b_ = static_cast<float>(get<double>(b));
                hilet a_ = static_cast<float>(get<double>(a));

                return hi::color(r_, g_, b_, a_);

            } else {
                throw parse_error(std::format("Expect all integers or all floating point numbers in a color, got {}.", data));
            }

        } else if (hilet *color_name = get_if<std::string>(data)) {
            hilet color_name_ = to_lower(*color_name);
            if (color_name_.starts_with("#")) {
                return color_from_sRGB(color_name_);

            } else {
                throw parse_error(std::format("Unable to parse color, got {}.", data));
            }
        } else {
            throw parse_error(std::format("Unable to parse color, got {}.", data));
        }
    }

    [[nodiscard]] hi::color parse_color(datum const& data, char const *object_name)
    {
        if (!data.contains(object_name)) {
            throw parse_error(std::format("Missing color '{}'", object_name));
        }

        hilet color_object = data[object_name];

        try {
            return parse_color_value(color_object);
        } catch (parse_error const&) {
            if (auto s = get_if<std::string>(color_object)) {
                return hi::color{semantic_color_from_string(*s)};
            } else {
                throw;
            }
        }
    }

    [[nodiscard]] std::vector<hi::color> parse_color_list(datum const& data, char const *object_name)
    {
        // Extract name
        if (!data.contains(object_name)) {
            throw parse_error(std::format("Missing color list '{}'", object_name));
        }

        hilet color_list_object = data[object_name];
        if (holds_alternative<datum::vector_type>(color_list_object) and not color_list_object.empty() and
            holds_alternative<datum::vector_type>(color_list_object[0])) {
            auto r = std::vector<hi::color>{};
            ssize_t i = 0;
            for (hilet& color : color_list_object) {
                try {
                    r.push_back(parse_color_value(color));
                } catch (parse_error const& e) {
                    throw parse_error(
                        std::format("Could not parse {}nd entry of color list '{}'\n{}", i + 1, object_name, e.what()));
                }
            }
            return r;

        } else {
            try {
                return {parse_color_value(data[object_name])};
            } catch (parse_error const& e) {
                throw parse_error(std::format("Could not parse color '{}'\n{}", object_name, e.what()));
            }
        }
    }

    [[nodiscard]] hi::text_style parse_text_style_value(datum const& data)
    {
        if (!holds_alternative<datum::map_type>(data)) {
            throw parse_error(std::format("Expect a text-style to be an object, got '{}'", data));
        }

        hilet family_id = find_font_family(parse_string(data, "family"));
        hilet font_size = parse_float(data, "size");

        auto variant = font_variant{};
        if (data.contains("weight")) {
            variant.set_weight(parse_font_weight(data, "weight"));
        } else {
            variant.set_weight(font_weight::regular);
        }

        if (data.contains("italic")) {
            variant.set_style(parse_bool(data, "italic") ? font_style::italic : font_style::normal);
        } else {
            variant.set_style(font_style::normal);
        }

        // resolve semantic color.
        hilet color = this->color(parse_color(data, "color"), 0);

        auto sub_styles = std::vector<text_sub_style>{};
        sub_styles.emplace_back(
            phrasing_mask::all, iso_639{}, iso_15924{}, family_id, variant, font_size, color, text_decoration{});
        return hi::text_style(sub_styles);
    }

    [[nodiscard]] font_weight parse_font_weight(datum const& data, char const *object_name)
    {
        if (!data.contains(object_name)) {
            throw parse_error(std::format("Missing '{}'", object_name));
        }

        hilet object = data[object_name];
        if (auto i = get_if<long long>(object)) {
            return font_weight_from_int(*i);
        } else if (auto s = get_if<std::string>(object)) {
            return font_weight_from_string(*s);
        } else {
            throw parse_error(std::format("Unable to parse font weight, got {}.", object.type_name()));
        }
    }

    [[nodiscard]] hi::text_style parse_text_style(datum const& data, char const *object_name)
    {
        // Extract name
        if (!data.contains(object_name)) {
            throw parse_error(std::format("Missing text-style '{}'", object_name));
        }

        hilet textStyleObject = data[object_name];
        try {
            return parse_text_style_value(textStyleObject);
        } catch (parse_error const& e) {
            throw parse_error(std::format("Could not parse text-style '{}'\n{}", object_name, e.what()));
        }
    }

    void parse(datum const& data)
    {
        hi_assert(holds_alternative<datum::map_type>(data));

        name = parse_string(data, "name");

        hilet mode_name = to_lower(parse_string(data, "mode"));
        if (mode_name == "light") {
            mode = theme_mode::light;
        } else if (mode_name == "dark") {
            mode = theme_mode::dark;
        } else {
            throw parse_error(std::format("Attribute 'mode' must be \"light\" or \"dark\", got \"{}\".", mode_name));
        }

        std::get<std::to_underlying(semantic_color::blue)>(_colors) = parse_color_list(data, "blue");
        std::get<std::to_underlying(semantic_color::green)>(_colors) = parse_color_list(data, "green");
        std::get<std::to_underlying(semantic_color::indigo)>(_colors) = parse_color_list(data, "indigo");
        std::get<std::to_underlying(semantic_color::orange)>(_colors) = parse_color_list(data, "orange");
        std::get<std::to_underlying(semantic_color::pink)>(_colors) = parse_color_list(data, "pink");
        std::get<std::to_underlying(semantic_color::purple)>(_colors) = parse_color_list(data, "purple");
        std::get<std::to_underlying(semantic_color::red)>(_colors) = parse_color_list(data, "red");
        std::get<std::to_underlying(semantic_color::teal)>(_colors) = parse_color_list(data, "teal");
        std::get<std::to_underlying(semantic_color::yellow)>(_colors) = parse_color_list(data, "yellow");

        std::get<std::to_underlying(semantic_color::gray)>(_colors) = parse_color_list(data, "gray");
        std::get<std::to_underlying(semantic_color::gray2)>(_colors) = parse_color_list(data, "gray2");
        std::get<std::to_underlying(semantic_color::gray3)>(_colors) = parse_color_list(data, "gray3");
        std::get<std::to_underlying(semantic_color::gray4)>(_colors) = parse_color_list(data, "gray4");
        std::get<std::to_underlying(semantic_color::gray5)>(_colors) = parse_color_list(data, "gray5");
        std::get<std::to_underlying(semantic_color::gray6)>(_colors) = parse_color_list(data, "gray6");

        std::get<std::to_underlying(semantic_color::foreground)>(_colors) = parse_color_list(data, "foreground-color");
        std::get<std::to_underlying(semantic_color::border)>(_colors) = parse_color_list(data, "border-color");
        std::get<std::to_underlying(semantic_color::fill)>(_colors) = parse_color_list(data, "fill-color");
        std::get<std::to_underlying(semantic_color::accent)>(_colors) = parse_color_list(data, "accent-color");
        std::get<std::to_underlying(semantic_color::text_select)>(_colors) = parse_color_list(data, "text-select-color");
        std::get<std::to_underlying(semantic_color::primary_cursor)>(_colors) = parse_color_list(data, "primary-cursor-color");
        std::get<std::to_underlying(semantic_color::secondary_cursor)>(_colors) =
            parse_color_list(data, "secondary-cursor-color");

        std::get<std::to_underlying(semantic_text_style::label)>(_text_styles) = parse_text_style(data, "label-style");
        std::get<std::to_underlying(semantic_text_style::small_label)>(_text_styles) =
            parse_text_style(data, "small-label-style");
        std::get<std::to_underlying(semantic_text_style::warning)>(_text_styles) = parse_text_style(data, "warning-label-style");
        std::get<std::to_underlying(semantic_text_style::error)>(_text_styles) = parse_text_style(data, "error-label-style");
        std::get<std::to_underlying(semantic_text_style::help)>(_text_styles) = parse_text_style(data, "help-label-style");
        std::get<std::to_underlying(semantic_text_style::placeholder)>(_text_styles) =
            parse_text_style(data, "placeholder-label-style");
        std::get<std::to_underlying(semantic_text_style::link)>(_text_styles) = parse_text_style(data, "link-label-style");

        _margin = narrow_cast<float>(parse_int(data, "margin"));
        _border_width = narrow_cast<float>(parse_int(data, "border-width"));
        _rounding_radius = narrow_cast<float>(parse_int(data, "rounding-radius"));
        _size = narrow_cast<float>(parse_int(data, "size"));
        _large_size = narrow_cast<float>(parse_int(data, "large-size"));
        _icon_size = narrow_cast<float>(parse_int(data, "icon-size"));
        _large_icon_size = narrow_cast<float>(parse_int(data, "large-icon-size"));
        _label_icon_size = narrow_cast<float>(parse_int(data, "label-icon-size"));

        _baseline_adjustment = std::ceil(std::get<std::to_underlying(semantic_text_style::label)>(_text_styles)->cap_height());
    }

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
