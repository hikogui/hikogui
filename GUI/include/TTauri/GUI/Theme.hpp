// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Text/FontDescription.hpp"
#include "TTauri/Text/TextStyle.hpp"
#include <array>

namespace TTauri::GUI {

/** Semantic colors.
*/
enum class ColorID {
    // Bright colors, used for accents in widgets or color-coding.
    // Blue is the default ON state color of a widget.
    Clear                   = 0x00, ///< Fully transparent
    Gray60                  = 0x01, ///< 60 % Gray compared to background
    Gray40                  = 0x02, ///< 40 % Gray compared to background
    Gray20                  = 0x03, ///< 20 % Gray compared to background
    Blue                    = 0x04, ///< Color of part or whole of the widget that is in on state
    Green                   = 0x05, ///< Color of part or whole of the widget that is in on state
    Indigo                  = 0x06, ///< Color of part or whole of the widget that is in on state
    Orange                  = 0x07, ///< Color of part or whole of the widget that is in on state
    Pink                    = 0x08, ///< Color of part or whole of the widget that is in on state
    Purple                  = 0x09, ///< Color of part or whole of the widget that is in on state
    Red                     = 0x0a, ///< Color of part or whole of the widget that is in on state
    Teal                    = 0x0b, ///< Color of part or whole of the widget that is in on state
    Yellow                  = 0x0c, ///< Color of part or whole of the widget that is in on state

    // Semantic colors.
    Background              = 0x0d, ///< Main background color of a window
    BackgroundSecondary     = 0x0e, ///< Background color of container within a window
    BackgroundTernary       = 0x0f, ///< Background color of container within a container
    Foreground              = 0x10, ///< Main text or icon color
    ForegroundSecondary     = 0x11, ///< Secondary text or icon color
    Fill                    = 0x12, ///< Color used to draw widgets and placeholder text.
    FillSecondary           = 0x13, ///< Secondary color to draw widgets
    Reserved1               = 0x14, ///< Reserved semantic color
    Reserved2               = 0x15, ///< Reserved semantic color
    Reserved3               = 0x16, ///< Reserved semantic color
    Reserved4               = 0x17, ///< Reserved semantic color

    // Custom colors are used by the application to draw custom widgets and text.
    Custom1                 = 0x18,
    Custom2                 = 0x19,
    Custom3                 = 0x1a,
    Custom4                 = 0x1b,
    Custom5                 = 0x1c,
    Custom6                 = 0x1d,
    Custom7                 = 0x1e,
    Custom8                 = 0x1f,

    max = Custom8
};

inline auto const ColorID_from_string_table = std::unordered_map<std::string,ColorID>{
    {"gray-60", ColorID::Gray60},
    {"gray-40", ColorID::Gray40},
    {"gray-20", ColorID::Gray20},
    {"blue", ColorID::Blue},
    {"green", ColorID::Green},
    {"indigo", ColorID::Indigo},
    {"orange", ColorID::Orange},
    {"pink", ColorID::Pink},
    {"purple", ColorID::Purple},
    {"red", ColorID::Red},
    {"teal", ColorID::Teal},
    {"yellow", ColorID::Yellow},
    {"background", ColorID::Background},
    {"background-secondary", ColorID::BackgroundSecondary},
    {"background-ternary", ColorID::BackgroundTernary},
    {"foreground", ColorID::Foreground},
    {"foreground-secondary", ColorID::ForegroundSecondary},
    {"fill", ColorID::FillSecondary},
    {"custom-1", ColorID::Custom1},
    {"custom-2", ColorID::Custom2},
    {"custom-3", ColorID::Custom3},
    {"custom-4", ColorID::Custom4},
    {"custom-5", ColorID::Custom5},
    {"custom-6", ColorID::Custom6},
    {"custom-7", ColorID::Custom7},
    {"custom-8", ColorID::Custom8},
};

enum class FontStyleID {
    Label,
    Text,
    Link,
    Heading,
    InputField,
    InputFieldPlaceholder,

    max = InputFieldPlaceholder
};

inline auto const FontStyleID_from_string_table = std::unordered_map<std::string,FontStyleID>{
    {"label", FontStyleID::Label},
    {"text", FontStyleID::Text},
    {"link", FontStyleID::Link},
    {"heading", FontStyleID::Heading},
    {"input-field", FontStyleID::InputField},
    {"input-field-placeholder", FontStyleID::InputFieldPlaceholder},
};




struct Theme {
    std::string name;

    /// IETF language tags.
    std::array<std::string,4> language_tags;

    /// 16 colors.
    std::array<wsRGBA,nr_items_v<ColorID>> color_palette;
    ColorID default_accent_color;

    std::array<TTauri::Text::TextStyle,nr_items_v<FontStyleID>> text_styles;
};

[[nodiscard]] Theme parse_theme(URL const &url);

}