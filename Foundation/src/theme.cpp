// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/JSON.hpp"
#include "TTauri/Foundation/theme.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri {


[[nodiscard]] font_style_index parse_theme_font_style_index(datum data)
{
    if (data.is_integer()) {
        let index = static_cast<int>(data);
        if (index < 0 || index > 7) {
            TTAURI_THROW(parse_error("Expect a font-style index to be an integer between 0 and 7, got {}", index));
        }

        return static_cast<font_style_index>(index);

    } else if (data.is_string()) {
        let key = static_cast<std::string>(data);

        let i = font_style_name_to_index_table.find(key);
        if (i == font_style_name_to_index_table.end()) {
            TTAURI_THROW(parse_error("Unknown font-style index {}"));
        } else {
            return i->second;
        }
    } else {
        TTAURI_THROW(parse_error("Expect a font-style index to be an integer or a string {}"));
    }
}

[[nodiscard]] color_index parse_theme_color_index(datum data)
{
    if (data.is_integer()) {
        let index = static_cast<int>(data);
        if (index < 0 || index > 0x1f) {
            TTAURI_THROW(parse_error("Expect a color index to be an integer between 0 and 31, got {}", index));
        }

        return static_cast<color_index>(index);

    } else if (data.is_string()) {
        let key = static_cast<std::string>(data);

        let i = color_name_to_index_table.find(key);
        if (i == color_name_to_index_table.end()) {
            TTAURI_THROW(parse_error("Unknown color index {}", key));
        } else {
            return i->second;
        }
    } else {
        TTAURI_THROW(parse_error("Expect a color index to be an integer or a string, got {}", data));
    }
}

[[nodiscard]] font_weight parse_theme_font_weight(datum data)
{
    if (data.is_integer()) {
        let index = static_cast<int>(data);
        if (index < 150) {
            return font_weight::Thin;
        } else if (index < 250) {
            return font_weight::ExtraLight;
        } else if (index < 350) {
            return font_weight::Light;
        } else if (index < 500) {
            return font_weight::Regular;
        } else if (index < 650) {
            return font_weight::SemiBold;
        } else if (index < 750) {
            return font_weight::Bold;
        } else if (index < 825) {
            return font_weight::ExtraBold;
        } else {
            return font_weight::Black;
        }

    } else if (data.is_string()) {
        let key = static_cast<std::string>(data);

        let i = font_weight_name_to_index_table.find(key);
        if (i == font_weight_name_to_index_table.end()) {
            TTAURI_THROW(parse_error("Unknown font-weight {}"));
        } else {
            return i->second;
        }
    } else {
        TTAURI_THROW(parse_error("Expect font-weight to be an integer or a string, got {}", data));
    }
}

[[nodiscard]] font_decoration parse_theme_decoration(datum data)
{
    if (data.is_string()) {
        let key = static_cast<std::string>(data);

        let i = font_decoration_name_to_index_table.find(key);
        if (i == font_decoration_name_to_index_table.end()) {
            TTAURI_THROW(parse_error("Unknown decoration {}"));
        } else {
            return i->second;
        }
    } else {
        TTAURI_THROW(parse_error("Expecting decoration attribute to have a string value, got {}", data));
    }
}

[[nodiscard]] static font_style parse_theme_font_style(datum const &map)
{
    if (!map.is_map()) {
        TTAURI_THROW(parse_error("Expect a font-styles to be an object, got type {}", map.type_name()));
    }

    auto r = font_style{};
    for (auto i = map.map_begin(); i != map.map_end(); ++i) {
        let key = static_cast<std::string>(i->first);

        if (key == "family") {
            if (!i->second.is_string()) {
                TTAURI_THROW(parse_error("Expect super-family attribute of a font style to be a string, got {}", i->second));
            }
            // Will never fail; automatic fall-back to "Noto".
            //r.set_family(Foundation_globals->font_book->find_family(static_cast<std::string>(i->second)));

        } else if (key == "serif") {
            if (!i->second.is_string()) {
                TTAURI_THROW(parse_error("Expect serif attribute of a font style to be a boolean, got {}", i->second));
            }
            r.set_serif(static_cast<bool>(i->second));

        } else if (key == "monospace") {
            if (!i->second.is_string()) {
                TTAURI_THROW(parse_error("Expect monospace attribute of a font style to be a boolean, got {}", i->second));
            }
            r.set_monospace(static_cast<bool>(i->second));

        } else if (key == "italic") {
            if (!i->second.is_string()) {
                TTAURI_THROW(parse_error("Expect italic attribute of a font style to be a boolean, got {}", i->second));
            }
            r.set_italic(static_cast<bool>(i->second));

        } else if (key == "condensed") {
            if (!i->second.is_string()) {
                TTAURI_THROW(parse_error("Expect condensed attribute of a font style to be a boolean, got {}", i->second));
            }
            r.set_condensed(static_cast<bool>(i->second));

        } else if (key == "weight") {
            r.set_weight(parse_theme_font_weight(i->second));

        } else if (key == "size") {
            if (!(i->second.is_string() || i->second.is_float())) {
                TTAURI_THROW(parse_error("Expect size attribute of a font style to be a numeric value, got {}", i->second));
            }
            r.set_condensed(static_cast<float>(i->second));

        } else if (key == "color") {
            r.set_color(parse_theme_color_index(i->second));

        } else if (key == "decoration") {
            r.set_decoration(parse_theme_decoration(i->second));

        } else {
            TTAURI_THROW(parse_error("Unknown font style attribute '{}'", key));
        }
    }

    return r;
}



void parse_theme_font_styles(theme &r, datum const &map)
{
    ttauri_assert(map.is_map());

    for (auto i = map.map_begin(); i != map.map_end(); ++i) {
        let key = static_cast<std::string>(i->first);

        try {
            let font_style_index = parse_theme_font_style_index(key);
            r.font_styles[static_cast<int>(font_style_index)] = parse_theme_font_style(i->second);

        } catch (error &e) {
            TTAURI_THROW(parse_error("Failed to assign font style {} in font-styles", key).caused_by(e));
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

    // Extract font-styles.
    if (!data.contains("font-styles")) {
        TTAURI_THROW(parse_error("Missing 'font-styles' section in theme '{}'", name));
    }
    let font_styles = data["font-styles"];
    if (!font_styles.is_map()) {
        TTAURI_THROW(parse_error("'font-styles' section in theme '{}' must be a JSON object, got {}.", name, font_styles.type_name()));
    }
    parse_theme_font_styles(r, font_styles);

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