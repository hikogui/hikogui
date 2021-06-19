// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme.hpp"
#include "../text/font_book.hpp"
#include "../application.hpp"
#include "../codec/JSON.hpp"
#include "../color/sRGB.hpp"
#include "../logger.hpp"
#include "../URL.hpp"

namespace tt {

theme::theme(URL const &url)
{
    try {
        tt_log_info("Parsing theme at {}", url);
        ttlet data = parse_JSON(url);
        parse(data);
    } catch (std::exception const &e) {
        throw io_error("{}: Could not load theme.\n{}", url, e.what());
    }
}

[[nodiscard]] std::string theme::parse_string(datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing '{}'", object_name);
    }
    ttlet object = data[object_name];
    if (!object.is_string()) {
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
    if (!object.is_numeric()) {
        throw parse_error("'{}' attribute must be a number, got {}.", object_name, object.type_name());
    }

    return static_cast<float>(object);
}

[[nodiscard]] bool theme::parse_bool(datum const &data, char const *object_name)
{
    if (!data.contains(object_name)) {
        throw parse_error("Missing '{}'", object_name);
    }

    ttlet object = data[object_name];
    if (!object.is_bool()) {
        throw parse_error("'{}' attribute must be a boolean, got {}.", object_name, object.type_name());
    }

    return static_cast<bool>(object);
}

[[nodiscard]] color theme::parse_color_value(datum const &data)
{
    if (data.is_vector()) {
        if (std::ssize(data) != 3 && std::ssize(data) != 4) {
            throw parse_error("Expect 3 or 4 values for a color, got {}.", data);
        }
        ttlet r = data[0];
        ttlet g = data[1];
        ttlet b = data[2];
        ttlet a = std::ssize(data) == 4 ? data[3] : (r.is_integer() ? datum{255} : datum{1.0});

        if (r.is_integer() && g.is_integer() && b.is_integer() && a.is_integer()) {
            return color_from_sRGB(
                static_cast<uint8_t>(r),
                static_cast<uint8_t>(g),
                static_cast<uint8_t>(b),
                static_cast<uint8_t>(a)
            );
        } else if (r.is_float() && g.is_float() && b.is_float() && a.is_float()) {
            return color(
                static_cast<float>(r),
                static_cast<float>(g),
                static_cast<float>(b),
                static_cast<float>(a)
            );
        } else {
            throw parse_error("Expect all integers or all floating point numbers in a color, got {}.", data);
        }

    } else if (data.is_string()) {
        ttlet color_name = to_lower(static_cast<std::string>(data));
        if (color_name.starts_with("#"s)) {
            return color_from_sRGB(color_name);

        } else if (color_name == "blue") { return blue;
        } else if (color_name == "green") { return green;
        } else if (color_name == "indigo") { return indigo;
        } else if (color_name == "orange") { return orange;
        } else if (color_name == "pink") { return pink;
        } else if (color_name == "purple") { return purple;
        } else if (color_name == "red") { return red;
        } else if (color_name == "teal") { return teal;
        } else if (color_name == "yellow") { return yellow;
        } else if (color_name == "foreground-color") { return foreground_color;
        } else if (color_name == "accent-color") { return accent_color;
        } else if (color_name == "text-select-color") { return text_select_color;
        } else if (color_name == "cursor-color") { return cursor_color;
        } else if (color_name == "incomplete-glyph-color") { return incomplete_glyph_color;
        } else {
            throw parse_error("Unable to parse color, got {}.", data);
        }
    } else {
        throw parse_error("Unable to parse color, got {}.", data);
    }
}

[[nodiscard]] color theme::parse_color(datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing color '{}'", object_name);
    }

    ttlet colorObject = data[object_name];
    try {
        return parse_color_value(colorObject);
    } catch (parse_error const &e) {
        throw parse_error("Could not parse color '{}'\n{}", object_name, e.what());
    }
}

[[nodiscard]] std::vector<color> theme::parse_color_list(datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing color list '{}'", object_name);
    }

    ttlet colorListObject = data[object_name];
    if (!colorListObject.is_vector()) {
        throw parse_error("Expecting color list '{}' to be a list of colors, got {}", object_name, colorListObject.type_name());
    }

    auto r = std::vector<color>{};
    ssize_t i = 0;
    for (auto it = colorListObject.vector_begin(); it != colorListObject.vector_end(); ++it, ++i) {
        try {
            r.push_back(parse_color_value(*it));
        } catch (parse_error const &e) {
            throw parse_error("Could not parse {}nd entry of color list '{}'\n{}", i + 1, name, e.what());
        }
    }
    return r;
}

[[nodiscard]] font_weight theme::parse_font_weight(datum const &data, char const *object_name)
{
    if (!data.contains(object_name)) {
        throw parse_error("Missing '{}'", object_name);
    }

    ttlet object = data[object_name];
    if (object.is_numeric()) {
        return font_weight_from_int(static_cast<int>(object));
    } else if (object.is_string()) {
        return font_weight_from_string(static_cast<std::string>(object));
    } else {
        throw parse_error("Unable to parse font weight, got {}.", object.type_name());
    }
}

[[nodiscard]] text_style theme::parse_text_style_value(datum const &data)
{
    if (!data.is_map()) {
        throw parse_error("Expect a text-style to be an object, got '{}'", data);
    }

    text_style r;

    r.family_id = font_book::global->find_family(parse_string(data, "family"));
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

[[nodiscard]] text_style theme::parse_text_style(datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing text-style '{}'", object_name);
    }

    ttlet textStyleObject = data[object_name];
    try {
        return parse_text_style_value(textStyleObject);
    } catch (parse_error const &e) {
        throw parse_error("Could not parse text-style '{}'\n{}", object_name, e.what());
    }
}

void theme::parse(datum const &data)
{
    tt_assert(data.is_map());

    this->name = parse_string(data, "name");

    ttlet mode_name = to_lower(parse_string(data, "mode"));
    if (mode_name == "light") {
        this->mode = theme_mode::light;
    } else if (mode_name == "dark") {
        this->mode = theme_mode::dark;
    } else {
        throw parse_error("Attribute 'mode' must be \"light\" or \"dark\", got \"{}\".", mode_name);
    }

    this->blue = parse_color(data, "blue");
    this->green = parse_color(data, "green");
    this->indigo = parse_color(data, "indigo");
    this->orange = parse_color(data, "orange");
    this->pink = parse_color(data, "pink");
    this->purple = parse_color(data, "purple");
    this->red = parse_color(data, "red");
    this->teal = parse_color(data, "teal");
    this->yellow = parse_color(data, "yellow");

    this->_gray_shades = parse_color_list(data, "gray-shades");
    this->_fill_shades = parse_color_list(data, "fill-shades");
    this->_border_shades = parse_color_list(data, "border-shades");

    this->foreground_color = parse_color(data, "foreground-color");
    this->accent_color = parse_color(data, "accent-color");
    this->text_select_color = parse_color(data, "text-select-color");
    this->cursor_color = parse_color(data, "cursor-color");
    this->incomplete_glyph_color = parse_color(data, "incomplete-glyph-color");

    this->label_style = parse_text_style(data, "label-style");
    this->small_label_style = parse_text_style(data, "small-label-style");
    this->warning_label_style = parse_text_style(data, "warning-label-style");
    this->error_label_style = parse_text_style(data, "error-label-style");
    this->help_label_style = parse_text_style(data, "help-label-style");
    this->placeholder_label_style = parse_text_style(data, "placeholder-label-style");
    this->link_label_style = parse_text_style(data, "link-label-style");
}

}
