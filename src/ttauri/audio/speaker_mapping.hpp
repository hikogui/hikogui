// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../architecture.hpp"
#include "../text/ttauri_icon.hpp"
#include <array>
#include <string>
#include <iostream>

namespace tt {

enum class speaker_mapping : uint64_t {
    /** Direct. speakers are not assigned, and no matrix-mixing is done.
    * Upper 32 bits contains the number of channels.
     */
    direct = 0x0'0000,

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

[[nodiscard]] constexpr bool to_bool(speaker_mapping const &rhs) noexcept
{
    return static_cast<bool>(static_cast<uint64_t>(rhs));
}

[[nodiscard]] constexpr bool holds_invariant(speaker_mapping const &rhs) noexcept
{
    auto rhs_ = static_cast<uint64_t>(rhs);
    return static_cast<uint32_t>(rhs_) ? (rhs_ >> 32) == 0 : (rhs_ >> 32) > 0;
}

[[nodiscard]] constexpr bool is_direct(speaker_mapping const &rhs) noexcept
{
    return static_cast<uint32_t>(static_cast<uint64_t>(rhs)) == 0;
}

[[nodiscard]] constexpr size_t num_channels(speaker_mapping const &rhs) noexcept
{
    if (is_direct(rhs)) {
        return static_cast<uint64_t>(rhs) >> 32;
    } else {
        return std::popcount(static_cast<uint64_t>(rhs));
    }
}

[[nodiscard]] std::string to_string(speaker_mapping rhs) noexcept;

inline std::ostream &operator<<(std::ostream &lhs, speaker_mapping rhs)
{
    return lhs << to_string(rhs);
}

struct speaker_mapping_info {
    speaker_mapping mapping;
    ttauri_icon icon;
    char const *name;
};

constexpr auto speaker_mappings = std::array{
    speaker_mapping_info{speaker_mapping::mono_1_0, ttauri_icon::mono_1_0, "Mono 1.0"},
    speaker_mapping_info{speaker_mapping::stereo_2_0, ttauri_icon::stereo_2_0, "Stereo 2.0"},
    speaker_mapping_info{speaker_mapping::stereo_2_1, ttauri_icon::stereo_2_1, "Stereo 2.1"},
    speaker_mapping_info{speaker_mapping::stereo_3_0, ttauri_icon::stereo_3_0, "Stereo 3.0"},
    speaker_mapping_info{speaker_mapping::stereo_3_1, ttauri_icon::stereo_3_1, "Stereo 3.1"},
    speaker_mapping_info{speaker_mapping::quad_4_0, ttauri_icon::quad_4_0, "Quad 4.0"},
    speaker_mapping_info{speaker_mapping::quad_side_4_0, ttauri_icon::quad_side_4_0, "Quad 4.0 (side)"},
    speaker_mapping_info{speaker_mapping::hexagonal_6_0, ttauri_icon::hexagonal_6_0, "Hexagonal 6.0"},
    speaker_mapping_info{speaker_mapping::hexagonal_6_1, ttauri_icon::hexagonal_6_1, "Hexagonal 6.1"},
    speaker_mapping_info{speaker_mapping::octagonal_8_0, ttauri_icon::octagonal_8_0, "Octagonal 8.0"},
    speaker_mapping_info{speaker_mapping::surround_3_0, ttauri_icon::surround_3_0, "Surround 3.0"},
    speaker_mapping_info{speaker_mapping::surround_4_0, ttauri_icon::surround_4_0, "Surround 4.0"},
    speaker_mapping_info{speaker_mapping::surround_4_1, ttauri_icon::surround_4_1, "Surround 4.1"},
    speaker_mapping_info{speaker_mapping::surround_5_0, ttauri_icon::surround_5_0, "Surround 5.0"},
    speaker_mapping_info{speaker_mapping::surround_5_1, ttauri_icon::surround_5_1, "Surround 5.1"},
    speaker_mapping_info{speaker_mapping::surround_7_0, ttauri_icon::surround_7_0, "Surround 7.0"},
    speaker_mapping_info{speaker_mapping::surround_7_1, ttauri_icon::surround_7_1, "Surround 7.1"},
    speaker_mapping_info{speaker_mapping::surround_9_0, ttauri_icon::surround_9_0, "Surround 9.0"},
    speaker_mapping_info{speaker_mapping::surround_9_1, ttauri_icon::surround_9_1, "Surround 9.1"},
    speaker_mapping_info{speaker_mapping::surround_11_0, ttauri_icon::surround_11_0, "Surround 11.0"},
    speaker_mapping_info{speaker_mapping::surround_11_1, ttauri_icon::surround_11_1, "Surround 11.1"},
    speaker_mapping_info{speaker_mapping::surround_side_5_0, ttauri_icon::surround_side_5_0, "Surround 5.0 (side)"},
    speaker_mapping_info{speaker_mapping::surround_side_5_1, ttauri_icon::surround_side_5_1, "Surround 5.1 (side)"},
    speaker_mapping_info{speaker_mapping::surround_side_6_0, ttauri_icon::surround_side_6_0, "Surround 6.0 (side)"},
    speaker_mapping_info{speaker_mapping::surround_side_6_1, ttauri_icon::surround_side_6_1, "Surround 6.1 (side)"},
    speaker_mapping_info{speaker_mapping::surround_side_7_0, ttauri_icon::surround_side_7_0, "Surround 7.0 (side)"},
    speaker_mapping_info{speaker_mapping::surround_side_7_1, ttauri_icon::surround_side_7_1, "Surround 7.1 (side)"},
    speaker_mapping_info{speaker_mapping::surround_wide_6_0, ttauri_icon::surround_wide_6_0, "Surround 6.0 (wide)"},
    speaker_mapping_info{speaker_mapping::surround_wide_6_1, ttauri_icon::surround_wide_6_1, "Surround 6.1 (wide)"},
    speaker_mapping_info{speaker_mapping::surround_wide_7_0, ttauri_icon::surround_wide_7_0, "Surround 7.0 (wide)"},
    speaker_mapping_info{speaker_mapping::surround_wide_7_1, ttauri_icon::surround_wide_7_1, "Surround 7.1 (wide)"},
    speaker_mapping_info{speaker_mapping::surround_atmos_5_1_4, ttauri_icon::surround_atmos_5_1_4, "Atmos 5.1.4"},
    speaker_mapping_info{speaker_mapping::surround_atmos_7_1_4, ttauri_icon::surround_atmos_7_1_4, "Atmos 7.1.4"},
};

}
