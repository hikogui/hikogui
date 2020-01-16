// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include <array>

namespace TTauri {

enum class font_weight {
    Thin = 0,       ///< 100: Thin / Hairline
    ExtraLight = 1, ///< 200: Ultra-light / Extra-light
    Light = 2,      ///< 300: Light
    Regular = 3,    ///< 400: Normal / Regular
    SemiBold = 4,   ///< 600: Medium / Semi-bold / Demi-bold
    Bold = 5,       ///< 700: Bold
    ExtraBold = 6,  ///< 800: Extra-bold / Ultra-bold
    Black = 7,      ///< 950: Heavy / Black / Extra-black / Ultra-black
};
constexpr int font_weight_bits = 3;
inline auto const font_weight_name_to_index_table = std::unordered_map<std::string,font_weight>{
    {"thin", font_weight::Thin},
    {"hairline", font_weight::Thin},
    {"ultra-light", font_weight::ExtraLight},
    {"extra-light", font_weight::ExtraLight},
    {"light", font_weight::Light},
    {"normal", font_weight::Regular},
    {"regular", font_weight::Regular},
    {"medium", font_weight::SemiBold},
    {"semi-bold", font_weight::SemiBold},
    {"demi-bold", font_weight::SemiBold},
    {"bold", font_weight::Bold},
    {"extra-bold", font_weight::ExtraBold},
    {"ultra-bold", font_weight::ExtraBold},
    {"heavy", font_weight::Black},
    {"black", font_weight::Black},
    {"extra-black", font_weight::Black},
    {"ultra-black", font_weight::Black},
};

[[nodiscard]] inline std::string to_string(font_weight const &x) noexcept {
    switch (x) {
    case font_weight::Thin: return "Thin";
    case font_weight::ExtraLight: return "ExtraLight";
    case font_weight::Light: return "Light";
    case font_weight::Regular: return "Regular";
    case font_weight::SemiBold: return "SemiBold";
    case font_weight::Bold: return "Bold";
    case font_weight::ExtraBold: return "ExtraBold";
    case font_weight::Black: return "Black";
    default: no_default;
    }
}

[[nodiscard]] inline char to_char(font_weight const &x) noexcept {
    switch (x) {
    case font_weight::Thin: return '1';
    case font_weight::ExtraLight: return '2';
    case font_weight::Light: return '3';
    case font_weight::Regular: return '4';
    case font_weight::SemiBold: return '5';
    case font_weight::Bold: return '6';
    case font_weight::ExtraBold: return '7';
    case font_weight::Black: return '8';
    default: no_default;
    }
}

inline std::ostream &operator<<(std::ostream &lhs, font_weight const &rhs) {
    return lhs << to_string(rhs);
}

[[nodiscard]] constexpr std::array<font_weight,64> font_weight_alternative_table_generator() noexcept
{
    std::array<font_weight,64> r = {font_weight::Regular};

    for (int w = 0; w < 8; ++w) {
        auto min_w = w;
        auto max_w = w;
        auto new_w = w;
        auto forward = false;

        for (int i = 0; i < 8; ++i) {
            r[(w << 3) | i] = static_cast<font_weight>(new_w);

            // Change direction to not overflow.
            if ((forward && max_w == 7) || (!forward && min_w == 0)) {
                forward = !forward;
            }

            if (forward) {
                ++max_w;
                new_w = max_w;
            } else {
                --min_w;
                new_w = min_w;
            }

            // Change direction to zig-zag.
            forward = !forward;
        }
    }
    return r;
}

constexpr std::array<font_weight,64> font_weight_alternative_table = font_weight_alternative_table_generator();

[[nodiscard]] constexpr font_weight font_weight_alterative(font_weight weight, int i) noexcept {
    ttauri_assume(i >= 0 && i < 8);
    auto w = static_cast<int>(weight);
    ttauri_assume(w >= 0 && w < 8);
    return font_weight_alternative_table[(w << 3) | i];
}

/** Describes how a grapheme should be underlined when rendering the text.
* It is carried with the grapheme and glyphs, so that the text render engine
* can draw the decoration after the text is shaped and in rendering-order
* (left to right) and, this makes it easier to correctly render the decoration
* of multiple glyphs in a single stroke.
*/
enum class font_decoration {
    None,
    Underline,
    WavyUnderline,
    StrikeThrough,
};
constexpr int font_decoration_bits = 2;
inline auto const font_decoration_name_to_index_table = std::unordered_map<std::string,font_decoration>{
    {"none", font_decoration::None},
    {"underline", font_decoration::Underline},
    {"wavy-underline", font_decoration::WavyUnderline},
    {"strike-through", font_decoration::StrikeThrough},
};

/** Describes how the background of a grapheme should drawn when rendering the text.
* It is carried with the grapheme and glyphs, so that the text render engine
* can draw the decoration after the text is shaped and in rendering-order
* (left to right) and, this makes it easier to correctly render the decoration
* of multiple glyphs in a single stroke.
*/
enum class font_background {
    None,
    Selected,
    SearchMatch,
    Reserved3,
};
constexpr int font_background_bits = 2;

/** Semantic colors.
*/
enum class color_index {
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
};
constexpr int color_index_bits = 5;
constexpr int color_index_size = 1 << 5;
inline auto const color_name_to_index_table = std::unordered_map<std::string,color_index>{
    {"gray-60", color_index::Gray60},
    {"gray-40", color_index::Gray40},
    {"gray-20", color_index::Gray20},
    {"blue", color_index::Blue},
    {"green", color_index::Green},
    {"indigo", color_index::Indigo},
    {"orange", color_index::Orange},
    {"pink", color_index::Pink},
    {"purple", color_index::Purple},
    {"red", color_index::Red},
    {"teal", color_index::Teal},
    {"yellow", color_index::Yellow},
    {"background", color_index::Background},
    {"background-secondary", color_index::BackgroundSecondary},
    {"background-ternary", color_index::BackgroundTernary},
    {"foreground", color_index::Foreground},
    {"foreground-secondary", color_index::ForegroundSecondary},
    {"fill", color_index::FillSecondary},
    {"custom-1", color_index::Custom1},
    {"custom-2", color_index::Custom2},
    {"custom-3", color_index::Custom3},
    {"custom-4", color_index::Custom4},
    {"custom-5", color_index::Custom5},
    {"custom-6", color_index::Custom6},
    {"custom-7", color_index::Custom7},
    {"custom-8", color_index::Custom8},
};

enum class font_style_index {
    Label,
    Text,
    Link,
    Heading,
    InputField,
    InputFieldPlaceholder,
};
constexpr ssize_t font_style_index_size = 2;
inline auto const font_style_name_to_index_table = std::unordered_map<std::string,font_style_index>{
    {"label", font_style_index::Label},
    {"text", font_style_index::Text},
    {"link", font_style_index::Link},
    {"heading", font_style_index::Heading},
    {"input-field", font_style_index::InputField},
    {"input-field-placeholder", font_style_index::InputFieldPlaceholder},
};

/** The font-style carries all the information needed to draw a grapheme in a certain style.
* The font-style is compressed in a single 32-bit integer to improve speed of comparison in
* the style table and for reduced storage when it is accompaniment with a grapheme or glyph.
*/
class font_style {
    uint32_t value;

    constexpr static int super_family_bits = 7;
    constexpr static int serif_bits = 1;
    constexpr static int monospace_bits = 1;
    constexpr static int italic_bits = 1;
    constexpr static int condensed_bits = 1;
    constexpr static int weight_bits = font_weight_bits;
    constexpr static int size_bits = 7;
    constexpr static int color_bits = color_index_bits;
    constexpr static int language_bits = 2;
    constexpr static int decoration_bits = font_decoration_bits;
    constexpr static int background_bits = font_background_bits;

    constexpr static int super_family_shift = (sizeof(value) * CHAR_BIT) - super_family_bits;
    constexpr static int serif_shift = super_family_shift - serif_bits;
    constexpr static int monospace_shift = serif_shift - monospace_bits;
    constexpr static int italic_shift = monospace_shift - italic_bits;
    constexpr static int condensed_shift = italic_shift - condensed_bits;
    constexpr static int weight_shift = condensed_shift - weight_bits;
    constexpr static int size_shift = weight_shift - size_bits;
    constexpr static int color_shift = size_shift - color_bits;
    constexpr static int language_shift = color_shift - language_bits;
    constexpr static int decoration_shift = language_shift - decoration_bits;
    constexpr static int background_shift = decoration_shift - background_bits;
    static_assert(background_shift >= 0);

    constexpr static uint64_t super_family_mask = (1ULL << super_family_bits) - 1;
    constexpr static uint64_t serif_mask = (1ULL << serif_bits) - 1;
    constexpr static uint64_t monospace_mask = (1ULL << monospace_bits) - 1;
    constexpr static uint64_t italic_mask = (1ULL << italic_bits) - 1;
    constexpr static uint64_t condensed_mask = (1ULL << condensed_bits) - 1;
    constexpr static uint64_t weight_mask = (1ULL << weight_bits) - 1;
    constexpr static uint64_t size_mask = (1ULL << size_bits) - 1;
    constexpr static uint64_t color_mask = (1ULL << color_bits) - 1;
    constexpr static uint64_t language_mask = (1ULL << language_bits) - 1;
    constexpr static uint64_t decoration_mask = (1ULL << decoration_bits) - 1;
    constexpr static uint64_t background_mask = (1ULL << background_bits) - 1;

public:
    /** Return the font super-family id.
    * The super font are the combination of fonts of multiple-styles
    * including those of serif/sans-serif, mono and slab types
    * weights and display sizes.
    */
    [[nodiscard]] int super_family_id() const noexcept {
        return (value >> super_family_shift) & super_family_mask;
    }

    font_style &set_super_family_id(int x) noexcept {
        ttauri_assert((x & super_family_mask) == x);
        value &= ~(super_family_mask << super_family_shift);
        value |= static_cast<uint64_t>(x) << super_family_shift;
        return *this;
    }

    [[nodiscard]] bool serif() const noexcept {
        return ((value >> serif_shift) & serif_mask) != 0;
    }

    font_style &set_serif(bool x) noexcept {
        value &= ~(1 << serif_shift);
        value |= static_cast<uint64_t>(x) << serif_shift;
        return *this;
    }

    [[nodiscard]] bool monospace() const noexcept {
        return ((value >> monospace_shift) & monospace_mask) != 0;
    }

    font_style &set_monospace(bool x) noexcept {
        value &= ~(1 << monospace_shift);
        value |= static_cast<uint64_t>(x) << monospace_shift;
        return *this;
    }

    [[nodiscard]] bool italic() const noexcept {
        return ((value >> italic_shift) & italic_mask) != 0;
    }

    font_style &set_italic(bool x) noexcept {
        value &= ~(1 << italic_shift);
        value |= static_cast<uint64_t>(x) << italic_shift;
        return *this;
    }

    [[nodiscard]] bool condensed() const noexcept {
        return ((value >> condensed_shift) & condensed_mask) != 0;
    }

    font_style &set_condensed(bool x) noexcept {
        value &= ~(1 << condensed_shift);
        value |= static_cast<uint64_t>(x) << condensed_shift;
        return *this;
    }

    [[nodiscard]] font_weight weight() const noexcept {
        return static_cast<font_weight>((value >> weight_shift) & weight_mask);
    }

    font_style &set_weight(font_weight x) noexcept {
        let x_ = static_cast<uint64_t>(x);
        ttauri_assert((x_ & weight_mask) == x_);
        value &= ~(weight_mask << weight_shift);
        value |= static_cast<uint64_t>(x_) << weight_shift;
        return *this;
    }

    /** Size of text in pt.
    */
    [[nodiscard]] float size() const noexcept {
        return static_cast<float>((value >> size_shift) & size_mask);
    }

    font_style &set_size(float x) noexcept {
        let x_ = static_cast<uint64_t>(x);
        ttauri_assert((x_ & size_mask) == x_);
        value &= ~(size_mask << size_shift);
        value |= static_cast<uint64_t>(x_) << size_shift;
        return *this;
    }

    [[nodiscard]] color_index color() const noexcept {
        return static_cast<color_index>((value >> color_shift) & color_mask);
    }

    font_style &set_color(color_index x) noexcept {
        let x_ = static_cast<uint64_t>(x);
        ttauri_assert((x_ & color_mask) == x_);
        value &= ~(color_mask << color_shift);
        value |= static_cast<uint64_t>(x_) << color_shift;
        return *this;
    }

    [[nodiscard]] int language() const noexcept {
        return (value >> language_shift) & language_mask;
    }

    font_style &set_language(int x) noexcept {
        ttauri_assert((x & language_mask) == x);
        value &= ~(language_mask << language_shift);
        value |= static_cast<uint64_t>(x) << language_shift;
        return *this;
    }

    [[nodiscard]] font_decoration decoration() const noexcept {
        return static_cast<font_decoration>((value >> decoration_shift) & decoration_mask);
    }

    font_style &set_decoration(font_decoration x) noexcept {
        let x_ = static_cast<uint64_t>(x);
        ttauri_assert((x_ & decoration_mask) == x_);
        value &= ~(decoration_mask << decoration_shift);
        value |= static_cast<uint64_t>(x_) << decoration_shift;
        return *this;
    }

    [[nodiscard]] font_background background() const noexcept {
        return static_cast<font_background>((value >> background_shift) & background_mask);
    }

    font_style &set_background(font_background x) noexcept {
        let x_ = static_cast<uint64_t>(x);
        ttauri_assert((x_ & background_mask) == x_);
        value &= ~(background_mask << background_shift);
        value |= static_cast<uint64_t>(x_) << background_shift;
        return *this;
    }
};


struct theme {
    std::string name;

    /// IETF language tags.
    std::array<std::string,4> language_tags;

    /// 16 colors.
    std::array<wsRGBA,color_index_size> color_palette;
    color_index default_accent_color;

    std::array<font_style,font_style_index_size> font_styles;
};

[[nodiscard]] theme parse_theme(URL const &url);

}