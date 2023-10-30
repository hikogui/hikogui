// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <random>

export module hikogui_char_maps_random_char;
import hikogui_utility;

export namespace hi {
inline namespace v1 {

char32_t random_char() noexcept
{
    static auto rand = std::mt19937();
    static auto size_dist = std::uniform_int_distribution(0, 99);
    static auto ascii_dist = std::uniform_int_distribution(0, 0x7f);
    static auto latin_dist = std::uniform_int_distribution(0x80, 0x7ff);
    static auto basic_dist = std::uniform_int_distribution(0x800, 0xf7ff); // without surrogates
    static auto full_dist = std::uniform_int_distribution(0x01'0000, 0x10'ffff);

    auto s = size_dist(rand);
    if (s < 90) {
        return char_cast<char32_t>(ascii_dist(rand));

    } else if (s < 95) {
        return char_cast<char32_t>(latin_dist(rand));

    } else if (s < 98) {
        auto c = char_cast<char32_t>(basic_dist(rand));
        if (c >= 0xd800 and c < 0xe000) {
            c += 0x800;
        }
        return c;

    } else {
        return char_cast<char32_t>(full_dist(rand));
    }
}

}} // namespace hi::inline v1
