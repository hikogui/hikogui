// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../exception.hpp"
#include "../strings.hpp"
#include "../concepts.hpp"
#include <string>
#include <unordered_map>
#include <ostream>
#include <array>

namespace hi::inline v1 {

enum class font_weight {
    Thin, ///< 100: Thin / Hairline
    ExtraLight, ///< 200: Ultra-light / Extra-light
    Light, ///< 300: Light
    Regular, ///< 400: Normal / Regular
    Medium, ///< 500: Medium
    SemiBold, ///< 600: Semi-bold / Demi-bold
    Bold, ///< 700: Bold
    ExtraBold, ///< 800: Extra-bold / Ultra-bold
    Black, ///< 900: Heavy / Black
    ExtraBlack, ///< 950: Extra-black / Ultra-black
};

inline hilet font_weight_from_string_table = std::unordered_map<std::string, font_weight>{
    {"thin", font_weight::Thin},
    {"hairline", font_weight::Thin},
    {"ultra-light", font_weight::ExtraLight},
    {"ultra light", font_weight::ExtraLight},
    {"extra-light", font_weight::ExtraLight},
    {"extra light", font_weight::ExtraLight},
    {"light", font_weight::Light},
    {"normal", font_weight::Regular},
    {"regular", font_weight::Regular},
    {"medium", font_weight::Medium},
    {"semi-bold", font_weight::SemiBold},
    {"semi bold", font_weight::SemiBold},
    {"demi-bold", font_weight::SemiBold},
    {"demi bold", font_weight::SemiBold},
    {"bold", font_weight::Bold},
    {"extra-bold", font_weight::ExtraBold},
    {"extra bold", font_weight::ExtraBold},
    {"ultra-bold", font_weight::ExtraBold},
    {"ultra bold", font_weight::ExtraBold},
    {"heavy", font_weight::Black},
    {"black", font_weight::Black},
    {"extra-black", font_weight::ExtraBlack},
    {"ultra-black", font_weight::ExtraBlack},
};

/** Convert a font weight value between 50 and 1000 to a font weight.
 */
[[nodiscard]] constexpr font_weight font_weight_from_int(numeric_integral auto rhs)
{
    if (rhs < 50 || rhs > 1000) {
        throw parse_error(std::format("Unknown font-weight {}", rhs));
    }
    return static_cast<font_weight>(((rhs + 50) / 100) - 1);
}

[[nodiscard]] inline font_weight font_weight_from_string(std::string_view rhs)
{
    hilet i = font_weight_from_string_table.find(to_lower(rhs));
    if (i == font_weight_from_string_table.end()) {
        throw parse_error(std::format("Unknown font-weight {}", rhs));
    }
    return i->second;
}

[[nodiscard]] constexpr char const *to_const_string(font_weight const &x) noexcept
{
    switch (x) {
    case font_weight::Thin: return "Thin";
    case font_weight::ExtraLight: return "ExtraLight";
    case font_weight::Light: return "Light";
    case font_weight::Regular: return "Regular";
    case font_weight::Medium: return "Medium";
    case font_weight::SemiBold: return "SemiBold";
    case font_weight::Bold: return "Bold";
    case font_weight::ExtraBold: return "ExtraBold";
    case font_weight::Black: return "Black";
    case font_weight::ExtraBlack: return "ExtraBlack";
    default: hi_no_default();
    }
}

[[nodiscard]] inline std::string to_string(font_weight const &x) noexcept
{
    return to_const_string(x);
}

[[nodiscard]] inline char to_char(font_weight const &x) noexcept
{
    hilet x_ = static_cast<int>(x);
    hi_axiom(x_ >= 0 && x_ <= 9);
    return static_cast<char>('0' + x_);
}

[[nodiscard]] constexpr int to_int(font_weight const &x) noexcept
{
    hilet x_ = (static_cast<int>(x) + 1) * 100;
    return (x_ == 1000) ? 950 : x_;
}

inline std::ostream &operator<<(std::ostream &lhs, font_weight const &rhs)
{
    return lhs << to_string(rhs);
}

inline bool almost_equal(font_weight const &lhs, font_weight const &rhs) noexcept
{
    // Check only if it is bold or not.
    return (lhs > font_weight::Medium) == (rhs > font_weight::Medium);
}

[[nodiscard]] constexpr auto font_weight_alternative_table_generator() noexcept
{
    std::array<font_weight, 100> r = {font_weight::Regular};

    for (auto w = 0_uz; w < 10_uz; ++w) {
        auto min_w = w;
        auto max_w = w;
        auto new_w = w;
        auto forward = false;

        for (auto i = 0_uz; i < 10_uz; ++i) {
            r[w * 10_uz + i] = static_cast<font_weight>(new_w);

            // Change direction to not overflow.
            if ((forward and max_w == 9_uz) or (not forward and min_w == 0_uz)) {
                forward = not forward;
            }

            new_w = forward ? ++max_w : --min_w;

            // Change direction to zig-zag.
            forward = not forward;
        }
    }
    return r;
}

constexpr auto font_weight_alternative_table = font_weight_alternative_table_generator();

[[nodiscard]] constexpr font_weight font_weight_alterative(font_weight weight, int i) noexcept
{
    hi_axiom(i >= 0 && i < 10);
    auto w = static_cast<int>(weight);
    hi_axiom(w >= 0 && w < 10);
    return font_weight_alternative_table[(w * 10) + i];
}

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::font_weight, CharT> : std::formatter<char const *, CharT> {
    auto format(hi::font_weight const &t, auto &fc)
    {
        return std::formatter<char const *, CharT>::format(hi::to_const_string(t), fc);
    }
};
