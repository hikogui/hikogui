// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Theme.hpp"
#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/JSON.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri::GUI {

Theme::Theme(URL const &url)
{
    try {
        let data = parseJSON(url);
        parse(data);
    } catch (error &e) {
        e.set<"url"_tag>(url);
        throw;
    }
}

[[nodiscard]] std::string Theme::parseString(datum const &data, char const *name)
{
    // Extract name
    if (!data.contains(name)) {
        TTAURI_THROW(parse_error("Missing '{}'", name));
    }
    let object = data[name];
    if (!object.is_string()) {
        TTAURI_THROW(parse_error("'{}' attribute must be a string, got {}.", name, object.type_name()));
    }
    return static_cast<std::string>(name);
}

[[nodiscard]] float Theme::parseFloat(datum const &data, char const *name)
{
    if (!data.contains(name)) {
        TTAURI_THROW(parse_error("Missing '{}'", name));
    }

    let object = data[name];
    if (!object.is_numeric()) {
        TTAURI_THROW(parse_error("'{}' attribute must be a number, got {}.", name, object.type_name()));
    }

    return static_cast<float>(object);
}

[[nodiscard]] bool Theme::parseBool(datum const &data, char const *name)
{
    if (!data.contains(name)) {
        TTAURI_THROW(parse_error("Missing '{}'", name));
    }

    let object = data[name];
    if (!object.is_bool()) {
        TTAURI_THROW(parse_error("'{}' attribute must be a boolean, got {}.", name, object.type_name()));
    }

    return static_cast<bool>(object);
}

[[nodiscard]] vec Theme::parseColorValue(datum const &data)
{
    if (data.is_vector()) {
        if (ssize(data) != 3 || ssize(data) != 4) {
            TTAURI_THROW(parse_error("Expect 3 or 4 values for a color, got {}.", data));
        }
        let r = data[0];
        let g = data[1];
        let b = data[2];
        let a = ssize(data) == 4 ? data[3] : (r.is_integer() ? datum{255} : datum{1.0});

        if (r.is_integer() && g.is_integer() && b.is_integer() && a.is_integer()) {
            return vec::colorFromSRGB(
                static_cast<int>(r),
                static_cast<int>(g),
                static_cast<int>(b),
                static_cast<int>(a)
            );
        } else if (r.is_float() && g.is_float() && b.is_float() && a.is_float()) {
            return vec::color(
                static_cast<float>(r),
                static_cast<float>(g),
                static_cast<float>(b),
                static_cast<float>(a)
            );
        } else {
            TTAURI_THROW(parse_error("Expect all integers or all floating point numbers in a color, got {}.", data));
        }

    } else if (data.is_string()) {
        let color_name = to_lower(static_cast<std::string>(data));
        if (starts_with(color_name, "#"s)) {
            return vec::colorFromSRGB(color_name);

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
            TTAURI_THROW(parse_error("Unable to parse color, got {}.", data));
        }
    } else {
        TTAURI_THROW(parse_error("Unable to parse color, got {}.", data));
    }
}

[[nodiscard]] vec Theme::parseColor(datum const &data, char const *name)
{
    // Extract name
    if (!data.contains(name)) {
        TTAURI_THROW(parse_error("Missing color '{}'", name));
    }

    let colorObject = data[name];
    try {
        return parseColorValue(colorObject);
    } catch (parse_error &e) {
        TTAURI_THROW(parse_error("Could not parse color '{}'", name).caused_by(e));
    }
}

[[nodiscard]] std::vector<vec> Theme::parseColorList(datum const &data, char const *name)
{
    // Extract name
    if (!data.contains(name)) {
        TTAURI_THROW(parse_error("Missing color list '{}'", name));
    }

    let colorListObject = data[name];
    if (!colorListObject.is_vector()) {
        TTAURI_THROW(parse_error("Expecting color list '{}' to be a list of colors, got {}", name, colorListObject.type_name()));
    }

    auto r = std::vector<vec>{};
    ssize_t i = 0;
    for (auto it = colorListObject.vector_begin(); it != colorListObject.vector_end(); ++it, ++i) {
        try {
            r.push_back(parseColorValue(*it));
        } catch (parse_error &e) {
            TTAURI_THROW(parse_error("Could not parse {}nd entry of color list '{}'", i + 1, name).caused_by(e));
        }
    }
    return r;
}

[[nodiscard]] Text::FontWeight Theme::parseFontWeight(datum const &data, char const *name)
{
    if (!data.contains(name)) {
        TTAURI_THROW(parse_error("Missing '{}'", name));
    }

    let object = data[name];
    if (object.is_numeric()) {
        return Text::FontWeight_from_int(static_cast<int>(object));
    } else if (object.is_string()) {
        return Text::FontWeight_from_string(static_cast<std::string>(object));
    } else {
        TTAURI_THROW(parse_error("Unable to parse font weight, got {}.", object.type_name()));
    }
}

[[nodiscard]] Text::TextStyle Theme::parseTextStyleValue(datum const &data)
{
    if (!data.is_map()) {
        TTAURI_THROW(parse_error("Expect a text-style to be an object, got '{}'", data));
    }

    Text::TextStyle r;

    r.family_id = Text::fontBook->find_family(parseString(data, "family"));
    r.size = parseFloat(data, "size");

    if (data.contains("weight")) {
        r.variant.set_weight(parseFontWeight(data, "weight"));
    } else {
        r.variant.set_weight(Text::FontWeight::Regular);
    }

    if (data.contains("italic")) {
        r.variant.set_italic(parseBool(data, "italic"));
    } else {
        r.variant.set_italic(false);
    }

    r.color = parseColor(data, "color");
    return r;
}

[[nodiscard]] Text::TextStyle Theme::parseTextStyle(datum const &data, char const *name)
{
    // Extract name
    if (!data.contains(name)) {
        TTAURI_THROW(parse_error("Missing text-style '{}'", name));
    }

    let textStyleObject = data[name];
    try {
        return parseTextStyleValue(textStyleObject);
    } catch (parse_error &e) {
        TTAURI_THROW(parse_error("Could not parse text-style '{}'", name).caused_by(e));
    }
}

void Theme::parse(datum const &data)
{
    ttauri_assert(data.is_map());

    this->name = parseString(data, "name");

    let mode = to_lower(parseString(data, "mode"));
    if (mode == "light") {
        this->mode = ThemeMode::Light;
    } else if (mode == "dark") {
        this->mode = ThemeMode::Dark;
    } else {
        TTAURI_THROW(parse_error("Attribute 'mode' must be \"light\" or \"dark\", got \"{}\".", mode));
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

    this->foregroundColor = parseColor(data, "foreground-color");
    this->accentColor = parseColor(data, "accent-color");
    this->textSelectColor = parseColor(data, "text-select-color");
    this->cursorColor = parseColor(data, "cursor-color");
    this->incompleteGlyphColor = parseColor(data, "incomplete-glyph-color");

    this->labelStyle = parseTextStyle(data, "label-style");
    this->warningLabelStyle = parseTextStyle(data, "warning-label-style");
    this->errorLabelStyle = parseTextStyle(data, "error-label-style");
    this->helpLabelStyle = parseTextStyle(data, "help-label-style");
    this->placeholderLabelStyle = parseTextStyle(data, "placeholder-label-style");
    this->linkLabelStyle = parseTextStyle(data, "link-label-style");
}

}
