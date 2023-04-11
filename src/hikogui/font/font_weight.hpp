// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../strings.hpp"
#include <string>
#include <unordered_map>
#include <ostream>
#include <array>

namespace hi::inline v1 {

enum class font_weight {
    thin, ///< 100: Thin / Hairline
    extra_light, ///< 200: Ultra-light / Extra-light
    light, ///< 300: Light
    regular, ///< 400: Normal / Regular
    medium, ///< 500: Medium
    semi_bold, ///< 600: Semi-bold / Demi-bold
    bold, ///< 700: Bold
    extra_bold, ///< 800: Extra-bold / Ultra-bold
    black, ///< 900: Heavy / Black
    extra_black, ///< 950: Extra-black / Ultra-black
};

// clang-format off
constexpr auto font_weight_metadata = enum_metadata{
    font_weight::thin, "thin",
    font_weight::extra_light, "extra-light",
    font_weight::light, "light",
    font_weight::regular, "regular",
    font_weight::medium, "medium",
    font_weight::semi_bold, "semi-bold",
    font_weight::bold, "bold",
    font_weight::extra_bold, "extra-bold",
    font_weight::black, "black",
    font_weight::extra_black, "extra-black",
};
// clang-format on

/** Convert a font weight value between 50 and 1000 to a font weight.
 */
[[nodiscard]] constexpr font_weight font_weight_from_int(numeric_integral auto rhs)
{
    if (rhs < 50 || rhs > 1000) {
        throw parse_error(std::format("Unknown font-weight {}", rhs));
    }
    return static_cast<font_weight>(((rhs + 50) / 100) - 1);
}

[[nodiscard]] constexpr font_weight font_weight_from_string(std::string_view rhs)
{
    try {
        return font_weight_metadata.at(rhs);
    } catch (...) {
        throw parse_error(std::format("Unknown font-weight {}", rhs));
    }
}

[[nodiscard]] constexpr std::string_view to_string_view(font_weight const& x) noexcept
{
    return font_weight_metadata[x];
}

[[nodiscard]] constexpr std::string to_string(font_weight const& x) noexcept
{
    return std::string{to_string_view(x)};
}

[[nodiscard]] constexpr char to_char(font_weight const &x) noexcept
{
    hilet x_ = static_cast<int>(x);
    hi_axiom(x_ >= 0 && x_ <= 9);
    return char_cast<char>('0' + x_);
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

constexpr bool almost_equal(font_weight const &lhs, font_weight const &rhs) noexcept
{
    // Check only if it is bold or not.
    return (lhs > font_weight::medium) == (rhs > font_weight::medium);
}

[[nodiscard]] constexpr auto font_weight_alternative_table_generator() noexcept
{
    std::array<font_weight, 100> r = {font_weight::regular};

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
struct std::formatter<hi::font_weight, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::font_weight const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(hi::to_string_view(t), fc);
    }
};
