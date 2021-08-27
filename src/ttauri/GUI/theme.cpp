// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme.hpp"
#include "theme_book.hpp"
#include "../text/font_book.hpp"
#include "../codec/JSON.hpp"
#include "../color/sRGB.hpp"
#include "../logger.hpp"
#include "../URL.hpp"

namespace tt {

theme::theme(tt::font_book const &font_book, URL const &url)
{
    try {
        tt_log_info("Parsing theme at {}", url);
        ttlet data = parse_JSON(url);
        parse(font_book, data);
    } catch (std::exception const &e) {
        throw io_error("{}: Could not load theme.\n{}", url, e.what());
    }
}

[[nodiscard]] tt::color theme::color(theme_color theme_color, ssize_t nesting_level) const noexcept
{
    ttlet theme_color_i = static_cast<size_t>(theme_color);
    tt_axiom(theme_color_i < std::size(_colors));

    ttlet &shades = _colors[theme_color_i];
    tt_axiom(std::ssize(shades) > 0);

    nesting_level = std::max(ssize_t{0}, nesting_level);
    return shades[nesting_level % std::ssize(shades)];
}

[[nodiscard]] tt::text_style const &theme::text_style(theme_text_style theme_text_style) const noexcept
{
    ttlet theme_text_style_i = static_cast<size_t>(theme_text_style);
    tt_axiom(theme_text_style_i < std::size(_text_styles));

    return _text_styles[theme_text_style_i];
}

[[nodiscard]] std::string theme::parse_string(datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing '{}'", object_name);
    }
    ttlet object = data[object_name];
    if (!holds_alternative<std::string>(object)) {
        throw parse_error("'{}' attribute must be a string, got {}.", object_name, object.type_name());
    }
    return static_cast<std::string>(object);
}

[[nodiscard]] float theme::parse_float(datum const &data, char const *object_name)
{
    if (!data.contains(object_name)) {
        throw parse_error("Missing '{}'", object_name);
    }

    ttlet object = data[object_name];
    if (auto f = get_if<double>(object)) {
        return static_cast<float>(*f);
    } else {
        throw parse_error("'{}' attribute must be a number, got {}.", object_name, object.type_name());
    }
}

[[nodiscard]] bool theme::parse_bool(datum const &data, char const *object_name)
{
    if (!data.contains(object_name)) {
        throw parse_error("Missing '{}'", object_name);
    }

    ttlet object = data[object_name];
    if (!holds_alternative<bool>(object)) {
        throw parse_error("'{}' attribute must be a boolean, got {}.", object_name, object.type_name());
    }

    return static_cast<bool>(object);
}

[[nodiscard]] color theme::parse_color_value(datum const &data)
{
    if (holds_alternative<datum::vector_type>(data)) {
        if (std::ssize(data) != 3 && std::ssize(data) != 4) {
            throw parse_error("Expect 3 or 4 values for a color, got {}.", data);
        }
        ttlet r = data[0];
        ttlet g = data[1];
        ttlet b = data[2];
        ttlet a = std::ssize(data) == 4 ? data[3] : (holds_alternative<long long>(r) ? datum{255} : datum{1.0});

        if (holds_alternative<long long>(r) and holds_alternative<long long>(g) and holds_alternative<long long>(b) and
            holds_alternative<long long>(a)) {

            auto r_ = get<long long>(r);
            auto g_ = get<long long>(g);
            auto b_ = get<long long>(b);
            auto a_ = get<long long>(a);

            tt_parse_check(r_ >= 0 and r_ <= 255, "integer red-color value not within 0 and 255");
            tt_parse_check(g_ >= 0 and g_ <= 255, "integer green-color value not within 0 and 255");
            tt_parse_check(b_ >= 0 and b_ <= 255, "integer blue-color value not within 0 and 255");
            tt_parse_check(a_ >= 0 and a_ <= 255, "integer alpha-color value not within 0 and 255");

            return color_from_sRGB(
                static_cast<uint8_t>(r_), static_cast<uint8_t>(g_), static_cast<uint8_t>(b_), static_cast<uint8_t>(a_));

        } else if (
            holds_alternative<double>(r) and holds_alternative<double>(g) and holds_alternative<double>(b) and
            holds_alternative<double>(a)) {

            auto r_ = get<double>(r);
            auto g_ = get<double>(g);
            auto b_ = get<double>(b);
            auto a_ = get<double>(a);

            return tt::color(r_, g_, b_, a_);

        } else {
            throw parse_error("Expect all integers or all floating point numbers in a color, got {}.", data);
        }

    } else if (ttlet *color_name = get_if<std::string>(data)) {
        ttlet color_name_ = to_lower(*color_name);
        if (color_name_.starts_with("#")) {
            return color_from_sRGB(color_name_);

        } else {
            throw parse_error("Unable to parse color, got {}.", data);
        }
    } else {
        throw parse_error("Unable to parse color, got {}.", data);
    }
}

[[nodiscard]] tt::color theme::parse_color(datum const &data, char const *object_name)
{
    if (!data.contains(object_name)) {
        throw parse_error("Missing color '{}'", object_name);
    }

    ttlet color_object = data[object_name];

    try {
        return parse_color_value(color_object);
    } catch (parse_error const &) {
        if (auto s = get_if<std::string>(color_object)) {
            ttlet theme_color = theme_color_from_string(*s);
            return this->color(theme_color);
        } else {
            throw;
        }
    }
}

[[nodiscard]] std::vector<color> theme::parse_color_list(datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing color list '{}'", object_name);
    }

    ttlet color_list_object = data[object_name];
    if (holds_alternative<datum::vector_type>(color_list_object) and std::size(color_list_object) > 0 and
        holds_alternative<datum::vector_type>(color_list_object[0])) {

        auto r = std::vector<tt::color>{};
        ssize_t i = 0;
        for (ttlet &color : color_list_object) {
            try {
                r.push_back(parse_color_value(color));
            } catch (parse_error const &e) {
                throw parse_error("Could not parse {}nd entry of color list '{}'\n{}", i + 1, object_name, e.what());
            }
        }
        return r;

    } else {
        try {
            return {parse_color_value(data[object_name])};
        } catch (parse_error const &e) {
            throw parse_error("Could not parse color '{}'\n{}", object_name, e.what());
        }
    }
}

[[nodiscard]] font_weight theme::parse_font_weight(datum const &data, char const *object_name)
{
    if (!data.contains(object_name)) {
        throw parse_error("Missing '{}'", object_name);
    }

    ttlet object = data[object_name];
    if (auto i = get_if<long long>(object)) {
        return font_weight_from_int(*i);
    } else if (auto s = get_if<std::string>(object)) {
        return font_weight_from_string(*s);
    } else {
        throw parse_error("Unable to parse font weight, got {}.", object.type_name());
    }
}

[[nodiscard]] text_style theme::parse_text_style_value(tt::font_book const &font_book, datum const &data)
{
    if (!holds_alternative<datum::map_type>(data)) {
        throw parse_error("Expect a text-style to be an object, got '{}'", data);
    }

    tt::text_style r;

    r.family_id = font_book.find_family(parse_string(data, "family"));
    r.size = parse_float(data, "size");

    if (data.contains("weight")) {
        r.variant.set_weight(parse_font_weight(data, "weight"));
    } else {
        r.variant.set_weight(font_weight::Regular);
    }

    if (data.contains("italic")) {
        r.variant.set_italic(parse_bool(data, "italic"));
    } else {
        r.variant.set_italic(false);
    }

    r.color = parse_color(data, "color");
    return r;
}

[[nodiscard]] text_style theme::parse_text_style(tt::font_book const &font_book, datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing text-style '{}'", object_name);
    }

    ttlet textStyleObject = data[object_name];
    try {
        return parse_text_style_value(font_book, textStyleObject);
    } catch (parse_error const &e) {
        throw parse_error("Could not parse text-style '{}'\n{}", object_name, e.what());
    }
}

void theme::parse(tt::font_book const &font_book, datum const &data)
{
    tt_assert(holds_alternative<datum::map_type>(data));

    this->name = parse_string(data, "name");

    ttlet mode_name = to_lower(parse_string(data, "mode"));
    if (mode_name == "light") {
        this->mode = theme_mode::light;
    } else if (mode_name == "dark") {
        this->mode = theme_mode::dark;
    } else {
        throw parse_error("Attribute 'mode' must be \"light\" or \"dark\", got \"{}\".", mode_name);
    }

    std::get<static_cast<size_t>(theme_color::blue)>(this->_colors) = parse_color_list(data, "blue");
    std::get<static_cast<size_t>(theme_color::green)>(this->_colors) = parse_color_list(data, "green");
    std::get<static_cast<size_t>(theme_color::indigo)>(this->_colors) = parse_color_list(data, "indigo");
    std::get<static_cast<size_t>(theme_color::orange)>(this->_colors) = parse_color_list(data, "orange");
    std::get<static_cast<size_t>(theme_color::pink)>(this->_colors) = parse_color_list(data, "pink");
    std::get<static_cast<size_t>(theme_color::purple)>(this->_colors) = parse_color_list(data, "purple");
    std::get<static_cast<size_t>(theme_color::red)>(this->_colors) = parse_color_list(data, "red");
    std::get<static_cast<size_t>(theme_color::teal)>(this->_colors) = parse_color_list(data, "teal");
    std::get<static_cast<size_t>(theme_color::yellow)>(this->_colors) = parse_color_list(data, "yellow");

    std::get<static_cast<size_t>(theme_color::gray)>(this->_colors) = parse_color_list(data, "gray");
    std::get<static_cast<size_t>(theme_color::gray2)>(this->_colors) = parse_color_list(data, "gray2");
    std::get<static_cast<size_t>(theme_color::gray3)>(this->_colors) = parse_color_list(data, "gray3");
    std::get<static_cast<size_t>(theme_color::gray4)>(this->_colors) = parse_color_list(data, "gray4");
    std::get<static_cast<size_t>(theme_color::gray5)>(this->_colors) = parse_color_list(data, "gray5");
    std::get<static_cast<size_t>(theme_color::gray6)>(this->_colors) = parse_color_list(data, "gray6");

    std::get<static_cast<size_t>(theme_color::foreground)>(this->_colors) = parse_color_list(data, "foreground-color");
    std::get<static_cast<size_t>(theme_color::border)>(this->_colors) = parse_color_list(data, "border-color");
    std::get<static_cast<size_t>(theme_color::fill)>(this->_colors) = parse_color_list(data, "fill-color");
    std::get<static_cast<size_t>(theme_color::accent)>(this->_colors) = parse_color_list(data, "accent-color");
    std::get<static_cast<size_t>(theme_color::text_select)>(this->_colors) = parse_color_list(data, "text-select-color");
    std::get<static_cast<size_t>(theme_color::cursor)>(this->_colors) = parse_color_list(data, "cursor-color");
    std::get<static_cast<size_t>(theme_color::incomplete_glyph)>(this->_colors) =
        parse_color_list(data, "incomplete-glyph-color");

    std::get<static_cast<size_t>(theme_text_style::label)>(this->_text_styles) = parse_text_style(font_book, data, "label-style");
    std::get<static_cast<size_t>(theme_text_style::small_label)>(this->_text_styles) =
        parse_text_style(font_book, data, "small-label-style");
    std::get<static_cast<size_t>(theme_text_style::warning)>(this->_text_styles) =
        parse_text_style(font_book, data, "warning-label-style");
    std::get<static_cast<size_t>(theme_text_style::error)>(this->_text_styles) =
        parse_text_style(font_book, data, "error-label-style");
    std::get<static_cast<size_t>(theme_text_style::help)>(this->_text_styles) =
        parse_text_style(font_book, data, "help-label-style");
    std::get<static_cast<size_t>(theme_text_style::placeholder)>(this->_text_styles) =
        parse_text_style(font_book, data, "placeholder-label-style");
    std::get<static_cast<size_t>(theme_text_style::link)>(this->_text_styles) =
        parse_text_style(font_book, data, "link-label-style");
}

} // namespace tt
