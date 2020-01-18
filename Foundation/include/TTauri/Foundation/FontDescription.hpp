// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/UnicodeData.hpp"
#include "TTauri/Foundation/GlyphID.hpp"

namespace TTauri {

enum class FontWeight {
    Thin,       ///< 100: Thin / Hairline
    ExtraLight, ///< 200: Ultra-light / Extra-light
    Light,      ///< 300: Light
    Regular,    ///< 400: Normal / Regular
    Medium,     ///< 500: Medium
    SemiBold,   ///< 600: Semi-bold / Demi-bold
    Bold,       ///< 700: Bold
    ExtraBold,  ///< 800: Extra-bold / Ultra-bold
    Black,      ///< 900: Heavy / Black
    ExtraBlack, ///< 950: Extra-black / Ultra-black
};

inline auto const FontWeight_from_string_table = std::unordered_map<std::string,FontWeight>{
    {"thin", FontWeight::Thin},
    {"hairline", FontWeight::Thin},
    {"ultra-light", FontWeight::ExtraLight},
    {"ultra light", FontWeight::ExtraLight},
    {"extra-light", FontWeight::ExtraLight},
    {"extra light", FontWeight::ExtraLight},
    {"light", FontWeight::Light},
    {"normal", FontWeight::Regular},
    {"regular", FontWeight::Regular},
    {"medium", FontWeight::Medium},
    {"semi-bold", FontWeight::SemiBold},
    {"semi bold", FontWeight::SemiBold},
    {"demi-bold", FontWeight::SemiBold},
    {"demi bold", FontWeight::SemiBold},
    {"bold", FontWeight::Bold},
    {"extra-bold", FontWeight::ExtraBold},
    {"extra bold", FontWeight::ExtraBold},
    {"ultra-bold", FontWeight::ExtraBold},
    {"ultra bold", FontWeight::ExtraBold},
    {"heavy", FontWeight::Black},
    {"black", FontWeight::Black},
    {"extra-black", FontWeight::ExtraBlack},
    {"ultra-black", FontWeight::ExtraBlack},
};

/** Convert a font weight value between 50 and 1000 to a font weight.
 */
[[nodiscard]] constexpr FontWeight FontWeight_from_int(int rhs) {
    if (rhs < 50 || rhs > 1000) {
        TTAURI_THROW(parse_error("Unknown font-weight {}", rhs));
    }
    return static_cast<FontWeight>(((rhs + 50) / 100) - 1);
}

[[nodiscard]] inline FontWeight FontWeight_from_string(std::string_view rhs) {
    let i = FontWeight_from_string_table.find(to_lower(rhs));
    if (i == FontWeight_from_string_table.end()) {
        TTAURI_THROW(parse_error("Unknown font-weight {}", rhs));
    }
    return i->second;
}

[[nodiscard]] constexpr char const *to_const_string(FontWeight const &x) noexcept {
    switch (x) {
    case FontWeight::Thin: return "Thin";
    case FontWeight::ExtraLight: return "ExtraLight";
    case FontWeight::Light: return "Light";
    case FontWeight::Regular: return "Regular";
    case FontWeight::Medium: return "Medium";
    case FontWeight::SemiBold: return "SemiBold";
    case FontWeight::Bold: return "Bold";
    case FontWeight::ExtraBold: return "ExtraBold";
    case FontWeight::Black: return "Black";
    case FontWeight::ExtraBlack: return "ExtraBlack";
    default: no_default;
    }
}

[[nodiscard]] inline std::string to_string(FontWeight const &x) noexcept {
    return to_const_string(x);
}

[[nodiscard]] inline char to_char(FontWeight const &x) noexcept {
    let x_ = static_cast<int>(x);
    ttauri_assume(x_ >= 0 && x_ <= 9);
    return '0' + x_;
}

[[nodiscard]] constexpr int to_int(FontWeight const &x) noexcept {
    let x_ = (static_cast<int>(x) + 1) * 100;
    return (x_ == 1000) ? 950 : x_;
}

inline std::ostream &operator<<(std::ostream &lhs, FontWeight const &rhs) {
    return lhs << to_string(rhs);
}

[[nodiscard]] constexpr auto FontWeight_alternative_table_generator() noexcept
{
    std::array<FontWeight,100> r = {FontWeight::Regular};

    for (int w = 0; w < 8; ++w) {
        auto min_w = w;
        auto max_w = w;
        auto new_w = w;
        auto forward = false;

        for (int i = 0; i < 10; ++i) {
            r[(w << 3) | i] = static_cast<FontWeight>(new_w);

            // Change direction to not overflow.
            if ((forward && max_w == 9) || (!forward && min_w == 0)) {
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

constexpr auto FontWeight_alternative_table = FontWeight_alternative_table_generator();

[[nodiscard]] constexpr FontWeight FontWeight_alterative(FontWeight weight, int i) noexcept {
    ttauri_assume(i >= 0 && i < 10);
    auto w = static_cast<int>(weight);
    ttauri_assume(w >= 0 && w < 10);
    return FontWeight_alternative_table[(w * 10) + i];
}

/** A font variant is one of 16 different fonts that can be part of a family.
* It only contains the font-weight and if it is italic/oblique.
*
* monospace, serif, condensed, expanded & optical-size are all part of the font family.
*/
class FontVariant {
    uint8_t value;

public:
    constexpr static int max() { return 20; }
    constexpr static int half() { return max() / 2; }

    constexpr FontVariant(FontWeight weight, bool italic) noexcept : value(static_cast<uint8_t>(static_cast<int>(weight) + (italic ? half() : 0))) {}
    constexpr FontVariant() noexcept : FontVariant(FontWeight::Regular, false) {}
    constexpr FontVariant(FontWeight weight) noexcept : FontVariant(weight, false) {}
    constexpr FontVariant(bool italic) noexcept : FontVariant(FontWeight::Regular, italic) {}

    constexpr FontWeight weight() const noexcept {
        ttauri_assume(value < max());
        return static_cast<FontWeight>(value % half());
    }

    [[nodiscard]] constexpr bool italic() const noexcept {
        ttauri_assume(value < max());
        return value >= half();
    }

    constexpr FontVariant &set_weight(FontWeight rhs) noexcept {
        value = static_cast<uint8_t>(static_cast<int>(rhs) + (italic() ? half() : 0));
        ttauri_assume(value < max());
        return *this;
    }

    constexpr FontVariant &set_italic(bool rhs) noexcept {
        value = static_cast<int>(weight()) + (rhs ? half() : 0);
        ttauri_assume(value < max());
        return *this;
    }

    constexpr operator int () const noexcept {
        ttauri_assume(value < max());
        return value;
    }

    /** Get an alternative font variant.
     * @param i 0 is current value, 1 is best alternative, 15 is worst alternative.
     */
    constexpr FontVariant alternative(int i) const noexcept {
        ttauri_assume(i >= 0 && i < max());
        let w = FontWeight_alterative(weight(), i % half());
        let it = italic() == (i < half());
        return {w, it};
    }
};

struct FontDescription {
    URL url;
    std::string family_name;
    std::string sub_family_name;

    bool monospace = false;
    bool serif = false;
    bool italic = false;
    bool condensed = false;
    FontWeight weight = FontWeight::Regular;
    float optical_size = 12.0;

    UnicodeRanges unicode_ranges;

    float xHeight = 0.0;
    float HHeight = 0.0;

    [[nodiscard]] FontVariant font_variant() const noexcept {
        return {weight, italic};
    }

    [[nodiscard]] friend std::string to_string(FontDescription const &rhs) noexcept {
        return fmt::format("{} - {}: {}{}{}{}{} {} {}",
            rhs.family_name,
            rhs.sub_family_name,
            rhs.monospace ? 'M' : '_',
            rhs.serif ? 'S': '_',
            rhs.italic ? 'I': '_',
            rhs.condensed ? 'C': '_',
            to_char(rhs.weight),
            rhs.optical_size,
            rhs.unicode_ranges
        );
    }

    friend std::ostream &operator<<(std::ostream &lhs, FontDescription const &rhs) {
        return lhs << to_string(rhs);
    }
};


}
