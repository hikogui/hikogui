// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../exceptions.hpp"
#include <string>
#include <unordered_map>
#include <ostream>

namespace tt {

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

inline ttlet FontWeight_from_string_table = std::unordered_map<std::string,FontWeight>{
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
    ttlet i = FontWeight_from_string_table.find(to_lower(rhs));
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
    default: tt_no_default();
    }
}

[[nodiscard]] inline std::string to_string(FontWeight const &x) noexcept {
    return to_const_string(x);
}

[[nodiscard]] inline char to_char(FontWeight const &x) noexcept {
    ttlet x_ = static_cast<int>(x);
    tt_assume(x_ >= 0 && x_ <= 9);
    return static_cast<char>('0' + x_);
}

[[nodiscard]] constexpr int to_int(FontWeight const &x) noexcept {
    ttlet x_ = (static_cast<int>(x) + 1) * 100;
    return (x_ == 1000) ? 950 : x_;
}

inline std::ostream& operator<<(std::ostream& lhs, FontWeight const& rhs) {
    return lhs << to_string(rhs);
}

inline bool almost_equal(FontWeight const &lhs, FontWeight const &rhs) noexcept {
    // Check only if it is bold or not.
    return (lhs > FontWeight::Medium) == (rhs > FontWeight::Medium);
}

[[nodiscard]] constexpr auto FontWeight_alternative_table_generator() noexcept
{
    std::array<FontWeight,100> r = {FontWeight::Regular};

    for (int w = 0; w < 10; ++w) {
        auto min_w = w;
        auto max_w = w;
        auto new_w = w;
        auto forward = false;

        for (int i = 0; i < 10; ++i) {
            r[w * 10 + i] = static_cast<FontWeight>(new_w);

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
    tt_assume(i >= 0 && i < 10);
    auto w = static_cast<int>(weight);
    tt_assume(w >= 0 && w < 10);
    return FontWeight_alternative_table[(w * 10) + i];
}

}
