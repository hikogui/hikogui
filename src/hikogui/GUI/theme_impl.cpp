// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme.hpp"
#include "theme_book.hpp"
#include "../font/module.hpp"
#include "../codec/JSON.hpp"
#include "../color/module.hpp"
#include "../log.hpp"
#include <algorithm>

namespace hi::inline v1 {

theme::theme(hi::font_book const& font_book, std::filesystem::path const& path)
{
    try {
        hi_log_info("Parsing theme at {}", path.string());
        hilet data = parse_JSON(path);
        parse(font_book, data);
    } catch (std::exception const& e) {
        throw io_error(std::format("{}: Could not load theme.\n{}", path.string(), e.what()));
    }
}

[[nodiscard]] theme theme::transform(float new_dpi) const noexcept
{
    auto r = *this;

    hi_assert(new_dpi != 0.0f);
    hi_assert(dpi != 0.0f);
    hi_assert(scale != 0.0f);

    auto delta_scale = new_dpi / dpi;
    r.dpi = new_dpi;
    r.scale = delta_scale * scale;

    // Scale each size, and round so that everything will stay aligned on pixel boundaries.
    r._margin = round_cast<int>(delta_scale * _margin);
    r._border_width = round_cast<int>(delta_scale * _border_width);
    r._rounding_radius = round_cast<int>(delta_scale * _rounding_radius);
    r._size = round_cast<int>(delta_scale * _size);
    r._large_size = round_cast<int>(delta_scale * _large_size);
    r._icon_size = round_cast<int>(delta_scale * _icon_size);
    r._large_icon_size = round_cast<int>(delta_scale * _large_icon_size);
    r._label_icon_size = round_cast<int>(delta_scale * _label_icon_size);
    // Cap height is not rounded, since the text-shaper will align the text to sub-pixel boundaries.
    r._baseline_adjustment = ceil_cast<int>(delta_scale * _baseline_adjustment);

    return r;
}

[[nodiscard]] hi::color theme::color(hi::semantic_color original_color, ssize_t nesting_level) const noexcept
{
    hilet& shades = _colors[to_underlying(original_color)];
    hi_assert(not shades.empty());

    nesting_level = std::max(ssize_t{0}, nesting_level);
    return shades[nesting_level % ssize(shades)];
}

[[nodiscard]] hi::color theme::color(hi::color original_color, ssize_t nesting_level) const noexcept
{
    if (original_color.is_semantic()) {
        return color(static_cast<semantic_color>(original_color), nesting_level);
    } else {
        return original_color;
    }
}

[[nodiscard]] hi::text_style theme::text_style(semantic_text_style semantic_text_style) const noexcept
{
    return _text_styles[to_underlying(semantic_text_style)];
}

[[nodiscard]] hi::text_style theme::text_style(hi::text_style original_style) const noexcept
{
    if (original_style.is_semantic()) {
        return text_style(static_cast<semantic_text_style>(original_style));
    } else {
        return original_style;
    }
}

[[nodiscard]] std::string theme::parse_string(datum const& data, char const *object_name)
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

[[nodiscard]] float theme::parse_float(datum const& data, char const *object_name)
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
        throw parse_error(std::format("'{}' attribute must be a floating point number, got {}.", object_name, object.type_name()));
    }
}

[[nodiscard]] long long theme::parse_long_long(datum const& data, char const *object_name)
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

[[nodiscard]] int theme::parse_int(datum const& data, char const *object_name)
{
    hilet value = parse_long_long(data, object_name);
    if (value > std::numeric_limits<int>::max() or value < std::numeric_limits<int>::min()) {
        throw parse_error(std::format("'{}' attribute is out of range, got {}.", object_name, value));
    }
    return narrow_cast<int>(value);
}

[[nodiscard]] bool theme::parse_bool(datum const& data, char const *object_name)
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

[[nodiscard]] color theme::parse_color_value(datum const& data)
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

[[nodiscard]] hi::color theme::parse_color(datum const& data, char const *object_name)
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

[[nodiscard]] std::vector<color> theme::parse_color_list(datum const& data, char const *object_name)
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
                throw parse_error(std::format("Could not parse {}nd entry of color list '{}'\n{}", i + 1, object_name, e.what()));
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

[[nodiscard]] font_weight theme::parse_font_weight(datum const& data, char const *object_name)
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

[[nodiscard]] text_style theme::parse_text_style_value(hi::font_book const& font_book, datum const& data)
{
    if (!holds_alternative<datum::map_type>(data)) {
        throw parse_error(std::format("Expect a text-style to be an object, got '{}'", data));
    }

    hilet family_id = font_book.find_family(parse_string(data, "family"));
    hilet font_size = parse_float(data, "size");

    auto variant = font_variant{};
    if (data.contains("weight")) {
        variant.set_weight(parse_font_weight(data, "weight"));
    } else {
        variant.set_weight(font_weight::Regular);
    }

    if (data.contains("italic")) {
        variant.set_italic(parse_bool(data, "italic"));
    } else {
        variant.set_italic(false);
    }

    // resolve semantic color.
    hilet color = this->color(parse_color(data, "color"), 0);

    auto sub_styles = std::vector<text_sub_style>{};
    sub_styles.emplace_back(
        text_phrasing_mask::all, iso_639{}, iso_15924{}, family_id, variant, font_size, color, text_decoration{});
    return hi::text_style(sub_styles);
}

[[nodiscard]] text_style theme::parse_text_style(hi::font_book const& font_book, datum const& data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error(std::format("Missing text-style '{}'", object_name));
    }

    hilet textStyleObject = data[object_name];
    try {
        return parse_text_style_value(font_book, textStyleObject);
    } catch (parse_error const& e) {
        throw parse_error(std::format("Could not parse text-style '{}'\n{}", object_name, e.what()));
    }
}

void theme::parse(hi::font_book const& font_book, datum const& data)
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

    std::get<to_underlying(semantic_color::blue)>(_colors) = parse_color_list(data, "blue");
    std::get<to_underlying(semantic_color::green)>(_colors) = parse_color_list(data, "green");
    std::get<to_underlying(semantic_color::indigo)>(_colors) = parse_color_list(data, "indigo");
    std::get<to_underlying(semantic_color::orange)>(_colors) = parse_color_list(data, "orange");
    std::get<to_underlying(semantic_color::pink)>(_colors) = parse_color_list(data, "pink");
    std::get<to_underlying(semantic_color::purple)>(_colors) = parse_color_list(data, "purple");
    std::get<to_underlying(semantic_color::red)>(_colors) = parse_color_list(data, "red");
    std::get<to_underlying(semantic_color::teal)>(_colors) = parse_color_list(data, "teal");
    std::get<to_underlying(semantic_color::yellow)>(_colors) = parse_color_list(data, "yellow");

    std::get<to_underlying(semantic_color::gray)>(_colors) = parse_color_list(data, "gray");
    std::get<to_underlying(semantic_color::gray2)>(_colors) = parse_color_list(data, "gray2");
    std::get<to_underlying(semantic_color::gray3)>(_colors) = parse_color_list(data, "gray3");
    std::get<to_underlying(semantic_color::gray4)>(_colors) = parse_color_list(data, "gray4");
    std::get<to_underlying(semantic_color::gray5)>(_colors) = parse_color_list(data, "gray5");
    std::get<to_underlying(semantic_color::gray6)>(_colors) = parse_color_list(data, "gray6");

    std::get<to_underlying(semantic_color::foreground)>(_colors) = parse_color_list(data, "foreground-color");
    std::get<to_underlying(semantic_color::border)>(_colors) = parse_color_list(data, "border-color");
    std::get<to_underlying(semantic_color::fill)>(_colors) = parse_color_list(data, "fill-color");
    std::get<to_underlying(semantic_color::accent)>(_colors) = parse_color_list(data, "accent-color");
    std::get<to_underlying(semantic_color::text_select)>(_colors) = parse_color_list(data, "text-select-color");
    std::get<to_underlying(semantic_color::primary_cursor)>(_colors) = parse_color_list(data, "primary-cursor-color");
    std::get<to_underlying(semantic_color::secondary_cursor)>(_colors) = parse_color_list(data, "secondary-cursor-color");

    std::get<to_underlying(semantic_text_style::label)>(_text_styles) = parse_text_style(font_book, data, "label-style");
    std::get<to_underlying(semantic_text_style::small_label)>(_text_styles) =
        parse_text_style(font_book, data, "small-label-style");
    std::get<to_underlying(semantic_text_style::warning)>(_text_styles) =
        parse_text_style(font_book, data, "warning-label-style");
    std::get<to_underlying(semantic_text_style::error)>(_text_styles) = parse_text_style(font_book, data, "error-label-style");
    std::get<to_underlying(semantic_text_style::help)>(_text_styles) = parse_text_style(font_book, data, "help-label-style");
    std::get<to_underlying(semantic_text_style::placeholder)>(_text_styles) =
        parse_text_style(font_book, data, "placeholder-label-style");
    std::get<to_underlying(semantic_text_style::link)>(_text_styles) = parse_text_style(font_book, data, "link-label-style");

    _margin = parse_int(data, "margin");
    _border_width = parse_int(data, "border-width");
    _rounding_radius = parse_int(data, "rounding-radius");
    _size = parse_int(data, "size");
    _large_size = parse_int(data, "large-size");
    _icon_size = parse_int(data, "icon-size");
    _large_icon_size = parse_int(data, "large-icon-size");
    _label_icon_size = parse_int(data, "label-icon-size");

    _baseline_adjustment =
        narrow_cast<int>(std::ceil(std::get<to_underlying(semantic_text_style::label)>(_text_styles)->cap_height(font_book)));
}

} // namespace hi::inline v1
