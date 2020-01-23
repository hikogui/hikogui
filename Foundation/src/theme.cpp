// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/JSON.hpp"
#include "TTauri/Foundation/theme.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri {


TextStyle::TextStyle(std::string_view family_name, FontVariant variant, float size, wsRGBA color, TextDecoration decoration) :
    TextStyle(Foundation_globals->font_book->find_family(family_name), variant, size, color, decoration) {}

[[nodiscard]] FontStyleID parse_theme_text_style_index(datum data)
{
    if (data.is_integer()) {
        let index = static_cast<int>(data);
        if (index < 0 || index > 7) {
            TTAURI_THROW(parse_error("Expect a text-style index to be an integer between 0 and 7, got {}", index));
        }

        return static_cast<FontStyleID>(index);

    } else if (data.is_string()) {
        let key = static_cast<std::string>(data);

        let i = FontStyleID_from_string_table.find(key);
        if (i == FontStyleID_from_string_table.end()) {
            TTAURI_THROW(parse_error("Unknown text-style index {}"));
        } else {
            return i->second;
        }
    } else {
        TTAURI_THROW(parse_error("Expect a text-style index to be an integer or a string {}"));
    }
}

[[nodiscard]] ColorID parse_theme_color_index(datum data)
{
    if (data.is_integer()) {
        let index = static_cast<int>(data);
        if (index < 0 || index > 0x1f) {
            TTAURI_THROW(parse_error("Expect a color index to be an integer between 0 and 31, got {}", index));
        }

        return static_cast<ColorID>(index);

    } else if (data.is_string()) {
        let key = static_cast<std::string>(data);

        let i = ColorID_from_string_table.find(key);
        if (i == ColorID_from_string_table.end()) {
            TTAURI_THROW(parse_error("Unknown color id {}", key));
        } else {
            return i->second;
        }
    } else {
        TTAURI_THROW(parse_error("Expect a color index to be an integer or a string, got {}", data));
    }
}

[[nodiscard]] FontWeight parse_theme_font_weight(datum data)
{
    if (data.is_integer()) {
        let index = static_cast<int>(data);
        return FontWeight_from_int(index);

    } else if (data.is_string()) {
        let key = static_cast<std::string>(data);

        let i = FontWeight_from_string_table.find(key);
        if (i == FontWeight_from_string_table.end()) {
            TTAURI_THROW(parse_error("Unknown font-weight {}"));
        } else {
            return i->second;
        }
    } else {
        TTAURI_THROW(parse_error("Expect font-weight to be an integer or a string, got {}", data));
    }
}

[[nodiscard]] TextDecoration parse_theme_decoration(datum data)
{
    if (data.is_string()) {
        let key = static_cast<std::string>(data);

        let i = TextDecoration_from_string_table.find(key);
        if (i == TextDecoration_from_string_table.end()) {
            TTAURI_THROW(parse_error("Unknown decoration {}"));
        } else {
            return i->second;
        }
    } else {
        TTAURI_THROW(parse_error("Expecting decoration attribute to have a string value, got {}", data));
    }
}

[[nodiscard]] static TextStyle parse_theme_text_style(datum const &map)
{
    if (!map.is_map()) {
        TTAURI_THROW(parse_error("Expect a text-styles to be an object, got type {}", map.type_name()));
    }

    auto r = TextStyle{};
    for (auto i = map.map_begin(); i != map.map_end(); ++i) {
        let key = static_cast<std::string>(i->first);

        if (key == "family") {
            if (!i->second.is_string()) {
                TTAURI_THROW(parse_error("Expect super-family attribute of a font style to be a string, got {}", i->second));
            }
            // Will never fail; automatic fall-back to "Noto".
            //r.set_family(Foundation_globals->font_book->find_family(static_cast<std::string>(i->second)));

        } else if (key == "italic") {
            if (!i->second.is_bool()) {
                TTAURI_THROW(parse_error("Expect italic attribute of a font style to be a boolean, got {}", i->second));
            }
            r.variant.set_italic(static_cast<bool>(i->second));

        } else if (key == "weight") {
            r.variant.set_weight(parse_theme_font_weight(i->second));

        } else if (key == "size") {
            if (!(i->second.is_string() || i->second.is_float())) {
                TTAURI_THROW(parse_error("Expect size attribute of a font style to be a numeric value, got {}", i->second));
            }
            r.size = static_cast<float>(i->second);

        } else if (key == "color") {
            not_implemented;
            //r.color = parse_theme_color_index(i->second);

        } else if (key == "decoration") {
            r.decoration = parse_theme_decoration(i->second);

        } else {
            TTAURI_THROW(parse_error("Unknown font style attribute '{}'", key));
        }
    }

    return r;
}



void parse_theme_text_styles(theme &r, datum const &map)
{
    ttauri_assert(map.is_map());

    for (auto i = map.map_begin(); i != map.map_end(); ++i) {
        let key = static_cast<std::string>(i->first);

        try {
            let text_style_id = parse_theme_text_style_index(key);
            r.text_styles[static_cast<int>(text_style_id)] = parse_theme_text_style(i->second);

        } catch (error &e) {
            TTAURI_THROW(parse_error("Failed to assign font style {} in text-styles", key).caused_by(e));
        }
    }
}

[[nodiscard]] static wsRGBA parse_theme_color(datum const &vector)
{
    if (!vector.is_vector()) {
        TTAURI_THROW(parse_error("Expect a color to be an array, got type {}", vector.type_name()));
    }

    if (ssize(vector) != 3) {
        TTAURI_THROW(parse_error("Expect a color to be an array of three items, got {} items", ssize(vector)));
    }

    let red = vector[0];
    let green = vector[1];
    let blue = vector[2];

    if (red.is_integer() && green.is_integer() && blue.is_integer()) {
        // Gamma correct sRGBA values.
        let red_ = static_cast<uint8_t>(red);
        let green_ = static_cast<uint8_t>(green);
        let blue_ = static_cast<uint8_t>(blue);
        return wsRGBA{red_, green_, blue_};

    } else if (red.is_float() && green.is_float() && blue.is_float()) {
        // Wide gamut linear sRGBA values.
        let red_ = static_cast<float>(red);
        let green_ = static_cast<float>(green);
        let blue_ = static_cast<float>(blue);
        return wsRGBA{red_, green_, blue_};

    } else {
        TTAURI_THROW(parse_error("Expect a color to be an array of three floats or three integers"));
    }
}

void parse_theme_colors(theme &r, datum const &map)
{
    ttauri_assert(map.is_map());

    for (auto i = map.map_begin(); i != map.map_end(); ++i) {
        let key = static_cast<std::string>(i->first);

        try {
            let color_index = parse_theme_color_index(key);
            r.color_palette[static_cast<int>(color_index)] = parse_theme_color(i->second);

        } catch (error &e) {
            TTAURI_THROW(parse_error("Failed to assign color {} in color-palette", key).caused_by(e));
        }
    }
}

[[nodiscard]] static theme _parse_theme(URL const &url)
{
    auto r = theme{};

    let data = parseJSON(url);
    ttauri_assert(data.is_map());

    // Extract name
    if (!data.contains("name")) {
        TTAURI_THROW(parse_error("Missing 'name' in theme"));
    }
    let name = data["name"];
    if (!name.is_string()) {
        TTAURI_THROW(parse_error("'name' attribute in theme must be a JSON string, got {}.", name.type_name()));
    }
    r.name = static_cast<std::string>(name);

    // Extract color-palette
    if (!data.contains("color-palette")) {
        TTAURI_THROW(parse_error("Missing 'color-palette' section in theme '{}'", name));
    }
    let color_palette = data["color-palette"];
    if (!color_palette.is_map()) {
        TTAURI_THROW(parse_error("'color-palette' section in theme '{}' must be a JSON object, got {}.", name, color_palette.type_name()));
    }
    parse_theme_colors(r, color_palette);

    // Extract default color accent.
    if (!data.contains("default-accent-color")) {
        TTAURI_THROW(parse_error("Missing 'default-accent-color' attribute in theme '{}'", name));
    }
    r.default_accent_color = parse_theme_color_index(data["default-accent-color"]);

    // Extract text-styles.
    if (!data.contains("text-styles")) {
        TTAURI_THROW(parse_error("Missing 'text-styles' section in theme '{}'", name));
    }
    let text_styles = data["text-styles"];
    if (!text_styles.is_map()) {
        TTAURI_THROW(parse_error("'text-styles' section in theme '{}' must be a JSON object, got {}.", name, text_styles.type_name()));
    }
    parse_theme_text_styles(r, text_styles);

    return r;
}

[[nodiscard]] theme parse_theme(URL const &url)
{
    try {
        return _parse_theme(url);
    } catch (error &e) {
        e.set<"url"_tag>(url);
        throw;
    }
}

}