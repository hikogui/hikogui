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

[[nodiscard]] std::string theme::parseString(datum const &data, char const *object_name)
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

[[nodiscard]] float theme::parseFloat(datum const &data, char const *object_name)
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

[[nodiscard]] bool theme::parseBool(datum const &data, char const *object_name)
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

[[nodiscard]] color theme::parseColorValue(datum const &data)
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
        } else if (color_name == "foreground-color") { return foregroundColor;
        } else if (color_name == "accent-color") { return accentColor;
        } else if (color_name == "text-select-color") { return textSelectColor;
        } else if (color_name == "cursor-color") { return cursorColor;
        } else if (color_name == "incomplete-glyph-color") { return incompleteGlyphColor;
        } else {
            throw parse_error("Unable to parse color, got {}.", data);
        }
    } else {
        throw parse_error("Unable to parse color, got {}.", data);
    }
}

[[nodiscard]] color theme::parseColor(datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing color '{}'", object_name);
    }

    ttlet colorObject = data[object_name];
    try {
        return parseColorValue(colorObject);
    } catch (parse_error const &e) {
        throw parse_error("Could not parse color '{}'\n{}", object_name, e.what());
    }
}

[[nodiscard]] std::vector<color> theme::parseColorList(datum const &data, char const *object_name)
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
            r.push_back(parseColorValue(*it));
        } catch (parse_error const &e) {
            throw parse_error("Could not parse {}nd entry of color list '{}'\n{}", i + 1, name, e.what());
        }
    }
    return r;
}

[[nodiscard]] font_weight theme::parsefont_weight(datum const &data, char const *object_name)
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

[[nodiscard]] text_style theme::parsetext_styleValue(datum const &data)
{
    if (!data.is_map()) {
        throw parse_error("Expect a text-style to be an object, got '{}'", data);
    }

    text_style r;

    r.family_id = font_book::global->find_family(parseString(data, "family"));
    r.size = parseFloat(data, "size");

    if (data.contains("weight")) {
        r.variant.set_weight(parsefont_weight(data, "weight"));
    } else {
        r.variant.set_weight(font_weight::Regular);
    }

    if (data.contains("italic")) {
        r.variant.set_italic(parseBool(data, "italic"));
    } else {
        r.variant.set_italic(false);
    }

    r.color = parseColor(data, "color");
    return r;
}

[[nodiscard]] text_style theme::parsetext_style(datum const &data, char const *object_name)
{
    // Extract name
    if (!data.contains(object_name)) {
        throw parse_error("Missing text-style '{}'", object_name);
    }

    ttlet textStyleObject = data[object_name];
    try {
        return parsetext_styleValue(textStyleObject);
    } catch (parse_error const &e) {
        throw parse_error("Could not parse text-style '{}'\n{}", object_name, e.what());
    }
}

void theme::parse(datum const &data)
{
    tt_assert(data.is_map());

    this->name = parseString(data, "name");

    ttlet mode_name = to_lower(parseString(data, "mode"));
    if (mode_name == "light") {
        this->mode = theme_mode::light;
    } else if (mode_name == "dark") {
        this->mode = theme_mode::dark;
    } else {
        throw parse_error("Attribute 'mode' must be \"light\" or \"dark\", got \"{}\".", mode_name);
    }

    this->blue = parseColor(data, "blue");
    this->green = parseColor(data, "green");
    this->indigo = parseColor(data, "indigo");
    this->orange = parseColor(data, "orange");
    this->pink = parseColor(data, "pink");
    this->purple = parseColor(data, "purple");
    this->red = parseColor(data, "red");
    this->teal = parseColor(data, "teal");
    this->yellow = parseColor(data, "yellow");

    this->grayShades = parseColorList(data, "gray-shades");
    this->fillShades = parseColorList(data, "fill-shades");
    this->borderShades = parseColorList(data, "border-shades");

    this->foregroundColor = parseColor(data, "foreground-color");
    this->accentColor = parseColor(data, "accent-color");
    this->textSelectColor = parseColor(data, "text-select-color");
    this->cursorColor = parseColor(data, "cursor-color");
    this->incompleteGlyphColor = parseColor(data, "incomplete-glyph-color");

    this->labelStyle = parsetext_style(data, "label-style");
    this->smallLabelStyle = parsetext_style(data, "small-label-style");
    this->warningLabelStyle = parsetext_style(data, "warning-label-style");
    this->errorLabelStyle = parsetext_style(data, "error-label-style");
    this->helpLabelStyle = parsetext_style(data, "help-label-style");
    this->placeholderLabelStyle = parsetext_style(data, "placeholder-label-style");
    this->linkLabelStyle = parsetext_style(data, "link-label-style");
}

}
