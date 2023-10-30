// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <array>
#include <string>
#include <iostream>

export module hikogui_audio_speaker_mapping;
import hikogui_codec;
import hikogui_font;
import hikogui_utility;

export namespace hi { inline namespace v1 {

export enum class speaker_mapping : uint32_t {
    none = 0,
    front_left = 0x0'0001,
    front_right = 0x0'0002,
    front_center = 0x0'0004,
    low_frequency = 0x0'0008,
    back_left = 0x0'0010,
    back_right = 0x0'0020,
    front_left_of_center = 0x0'0040,
    front_right_of_center = 0x0'0080,
    back_center = 0x0'0100,
    side_left = 0x0'0200,
    side_right = 0x0'0400,
    top_center = 0x0'0800,
    top_front_left = 0x0'1000,
    top_front_center = 0x0'2000,
    top_front_right = 0x0'4000,
    top_back_left = 0x0'8000,
    top_back_center = 0x1'0000,
    top_back_right = 0x2'0000,

    // Standard
    mono_1_0 = front_center,
    stereo_2_0 = front_left | front_right,

    // Music configuration
    stereo_2_1 = stereo_2_0 | low_frequency,
    stereo_3_0 = stereo_2_0 | front_center,
    stereo_3_1 = stereo_3_0 | low_frequency,
    quad_4_0 = stereo_2_0 | back_left | back_right,
    quad_side_4_0 = stereo_2_0 | side_left | side_right,
    hexagonal_6_0 = quad_4_0 | front_center | back_center,
    hexagonal_6_1 = hexagonal_6_0 | low_frequency,
    octagonal_8_0 = hexagonal_6_0 | side_left | side_right,

    // Standard surround sound
    surround_3_0 = stereo_2_0 | back_center,
    surround_4_0 = surround_3_0 | front_center,
    surround_4_1 = surround_4_0 | low_frequency,
    surround_5_0 = quad_4_0 | front_center,
    surround_5_1 = surround_5_0 | low_frequency,
    surround_7_0 = surround_5_0 | side_left | side_right,
    surround_7_1 = surround_7_0 | low_frequency,
    surround_9_0 = surround_7_0 | top_front_left | top_front_right,
    surround_9_1 = surround_9_0 | low_frequency,
    surround_11_0 = surround_9_0 | front_left_of_center | front_right_of_center,
    surround_11_1 = surround_11_0 | low_frequency,

    // Surround sound with side speakers instead of left/right back speakers.
    surround_side_5_0 = quad_side_4_0 | front_center,
    surround_side_5_1 = surround_side_5_0 | low_frequency,
    surround_side_6_0 = surround_side_5_0 | back_center,
    surround_side_6_1 = surround_side_6_0 | low_frequency,
    surround_side_7_0 = surround_side_5_0 | front_left_of_center | front_right_of_center,
    surround_side_7_1 = surround_side_7_0 | low_frequency,

    // Surround sound with extra front speakers.
    surround_wide_6_0 = surround_4_0 | front_left_of_center | front_right_of_center,
    surround_wide_6_1 = surround_wide_6_0 | low_frequency,
    surround_wide_7_0 = surround_5_0 | front_left_of_center | front_right_of_center,
    surround_wide_7_1 = surround_wide_7_0 | low_frequency,

    // Surround with extra top speakers
    surround_atmos_5_1_4 = surround_5_1 | top_front_left | top_front_right | top_back_left | top_back_right,
    surround_atmos_7_1_4 = surround_7_1 | top_front_left | top_front_right | top_back_left | top_back_right,
};

export [[nodiscard]] constexpr bool to_bool(speaker_mapping const& rhs) noexcept
{
    return to_bool(std::to_underlying(rhs));
}

export [[nodiscard]] constexpr unsigned int popcount(speaker_mapping const &rhs) noexcept
{
    return std::popcount(std::to_underlying(rhs));
}

export [[nodiscard]] constexpr speaker_mapping operator|(speaker_mapping const &lhs, speaker_mapping const &rhs) noexcept
{
    return static_cast<speaker_mapping>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

export [[nodiscard]] constexpr speaker_mapping operator&(speaker_mapping const& lhs, speaker_mapping const& rhs) noexcept
{
    return static_cast<speaker_mapping>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

export constexpr speaker_mapping &operator|=(speaker_mapping &lhs, speaker_mapping const &rhs) noexcept
{
    return lhs = lhs | rhs;
}

export constexpr speaker_mapping &operator&=(speaker_mapping &lhs, speaker_mapping const &rhs) noexcept
{
    return lhs = lhs & rhs;
}

export template<>
struct pickle<speaker_mapping> {
    [[nodiscard]] datum encode(speaker_mapping const &rhs) const noexcept
    {
        return datum{narrow_cast<long long>(std::to_underlying(rhs))};
    }

    [[nodiscard]] speaker_mapping decode(long long rhs) const
    {
        hi_check(rhs >= 0, "Expect speaker mapping to be encoded as a natural number, got {}.", rhs);
        return static_cast<speaker_mapping>(rhs);
    }

    [[nodiscard]] speaker_mapping decode(datum const &rhs) const
    {
        if (auto *i = get_if<long long>(rhs)) {
            return decode(*i);
        } else {
            throw parse_error(std::format("Expect speaker mapping to be encoded as a integer, got {}", rhs));
        }
    }
};

export [[nodiscard]] std::string to_string(speaker_mapping rhs) noexcept{
    auto r = std::string{};

    if (to_bool(rhs & speaker_mapping::front_left)) {
        r += ",fl";
    }
    if (to_bool(rhs & speaker_mapping::front_right)) {
        r += ",fr";
    }
    if (to_bool(rhs & speaker_mapping::front_center)) {
        r += ",fc";
    }
    if (to_bool(rhs & speaker_mapping::low_frequency)) {
        r += ",lfe";
    }
    if (to_bool(rhs & speaker_mapping::back_left)) {
        r += ",bl";
    }
    if (to_bool(rhs & speaker_mapping::back_right)) {
        r += ",br";
    }
    if (to_bool(rhs & speaker_mapping::front_left_of_center)) {
        r += ",flc";
    }
    if (to_bool(rhs & speaker_mapping::front_right_of_center)) {
        r += ",frc";
    }
    if (to_bool(rhs & speaker_mapping::back_center)) {
        r += ",bc";
    }
    if (to_bool(rhs & speaker_mapping::side_left)) {
        r += ",sl";
    }
    if (to_bool(rhs & speaker_mapping::side_right)) {
        r += ",sr";
    }
    if (to_bool(rhs & speaker_mapping::top_center)) {
        r += ",tc";
    }
    if (to_bool(rhs & speaker_mapping::top_front_left)) {
        r += ",tfl";
    }
    if (to_bool(rhs & speaker_mapping::top_front_center)) {
        r += ",tfc";
    }
    if (to_bool(rhs & speaker_mapping::top_front_right)) {
        r += ",tfr";
    }
    if (to_bool(rhs & speaker_mapping::top_back_left)) {
        r += ",tbl";
    }
    if (to_bool(rhs & speaker_mapping::top_back_center)) {
        r += ",tbc";
    }
    if (to_bool(rhs & speaker_mapping::top_back_right)) {
        r += ",tbr";
    }

    if (r.empty()) {
        r += '[';
    } else {
        // replace the first comma.
        r[0] = '[';
    }

    r += ']';
    return r;
}

}} // namespace hi::inline v1
