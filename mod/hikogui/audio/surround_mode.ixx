// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstdint>
#include <coroutine>

export module hikogui_audio_surround_mode;
import hikogui_audio_speaker_mapping;
import hikogui_coroutine;
import hikogui_font;
import hikogui_l10n;
import hikogui_utility;

export namespace hi { inline namespace v1 {

export enum class surround_mode : uint64_t {
    none = 0,
    mono_1_0 = uint64_t{1} << 0,
    stereo_2_0 = uint64_t{1} << 1,

    // Music configuration
    stereo_2_1 = uint64_t{1} << 2,
    stereo_3_0 = uint64_t{1} << 3,
    stereo_3_1 = uint64_t{1} << 4,
    quad_4_0 = uint64_t{1} << 5,
    quad_side_4_0 = uint64_t{1} << 6,
    hexagonal_6_0 = uint64_t{1} << 7,
    hexagonal_6_1 = uint64_t{1} << 8,
    octagonal_8_0 = uint64_t{1} << 9,

    // Standard surround sound
    surround_3_0 = uint64_t{1} << 10,
    surround_4_0 = uint64_t{1} << 11,
    surround_4_1 = uint64_t{1} << 12,
    surround_5_0 = uint64_t{1} << 13,
    surround_5_1 = uint64_t{1} << 14,
    surround_7_0 = uint64_t{1} << 15,
    surround_7_1 = uint64_t{1} << 16,
    surround_9_0 = uint64_t{1} << 17,
    surround_9_1 = uint64_t{1} << 18,
    surround_11_0 = uint64_t{1} << 19,
    surround_11_1 = uint64_t{1} << 20,

    // Surround sound with side speakers instead of left/right back speakers.
    surround_side_5_0 = uint64_t{1} << 21,
    surround_side_5_1 = uint64_t{1} << 22,
    surround_side_6_0 = uint64_t{1} << 23,
    surround_side_6_1 = uint64_t{1} << 24,
    surround_side_7_0 = uint64_t{1} << 25,
    surround_side_7_1 = uint64_t{1} << 26,

    // Surround sound with extra front speakers.
    surround_wide_6_0 = uint64_t{1} << 27,
    surround_wide_6_1 = uint64_t{1} << 28,
    surround_wide_7_0 = uint64_t{1} << 29,
    surround_wide_7_1 = uint64_t{1} << 30,

    // Surround with extra top speakers
    surround_atmos_5_1_4 = uint64_t{1} << 31,
    surround_atmos_7_1_4 = uint64_t{1} << 32,
};

// clang-format off
export constexpr auto surround_mode_icons = enum_metadata{
    surround_mode::none, hikogui_icon::none_0_0,
    surround_mode::mono_1_0, hikogui_icon::mono_1_0,
    surround_mode::stereo_2_0, hikogui_icon::stereo_2_0,
    surround_mode::stereo_2_1, hikogui_icon::stereo_2_1,
    surround_mode::stereo_3_0, hikogui_icon::stereo_3_0,
    surround_mode::stereo_3_1, hikogui_icon::stereo_3_1,
    surround_mode::quad_4_0, hikogui_icon::quad_4_0,
    surround_mode::quad_side_4_0, hikogui_icon::quad_side_4_0,
    surround_mode::hexagonal_6_0, hikogui_icon::hexagonal_6_0,
    surround_mode::hexagonal_6_1, hikogui_icon::hexagonal_6_1,
    surround_mode::octagonal_8_0, hikogui_icon::octagonal_8_0,
    surround_mode::surround_3_0, hikogui_icon::surround_3_0,
    surround_mode::surround_4_0, hikogui_icon::surround_4_0,
    surround_mode::surround_4_1, hikogui_icon::surround_4_1,
    surround_mode::surround_5_0, hikogui_icon::surround_5_0,
    surround_mode::surround_5_1, hikogui_icon::surround_5_1,
    surround_mode::surround_7_0, hikogui_icon::surround_7_0,
    surround_mode::surround_7_1, hikogui_icon::surround_7_1,
    surround_mode::surround_9_0, hikogui_icon::surround_9_0,
    surround_mode::surround_9_1, hikogui_icon::surround_9_1,
    surround_mode::surround_11_0, hikogui_icon::surround_11_0,
    surround_mode::surround_11_1, hikogui_icon::surround_11_1,
    surround_mode::surround_side_5_0, hikogui_icon::surround_side_5_0,
    surround_mode::surround_side_5_1, hikogui_icon::surround_side_5_1,
    surround_mode::surround_side_6_0, hikogui_icon::surround_side_6_0,
    surround_mode::surround_side_6_1, hikogui_icon::surround_side_6_1,
    surround_mode::surround_side_7_0, hikogui_icon::surround_side_7_0,
    surround_mode::surround_side_7_1, hikogui_icon::surround_side_7_1,
    surround_mode::surround_wide_6_0, hikogui_icon::surround_wide_6_0,
    surround_mode::surround_wide_6_1, hikogui_icon::surround_wide_6_1,
    surround_mode::surround_wide_7_0, hikogui_icon::surround_wide_7_0,
    surround_mode::surround_wide_7_1, hikogui_icon::surround_wide_7_1,
    surround_mode::surround_atmos_5_1_4, hikogui_icon::surround_atmos_5_1_4,
    surround_mode::surround_atmos_7_1_4, hikogui_icon::surround_atmos_7_1_4,
};

export constexpr auto surround_mode_names = enum_metadata{
    surround_mode::none, "none",
    surround_mode::mono_1_0, "mono",
    surround_mode::stereo_2_0, "stereo",
    surround_mode::stereo_2_1, "stereo 2.1",
    surround_mode::stereo_3_0, "stereo 3.0",
    surround_mode::stereo_3_1, "stereo 3.1",
    surround_mode::quad_4_0, "quad",
    surround_mode::quad_side_4_0, "quad-side",
    surround_mode::hexagonal_6_0, "hexagonal",
    surround_mode::hexagonal_6_1, "hexagonal 6.1",
    surround_mode::octagonal_8_0, "octagonal",
    surround_mode::surround_3_0, "surround 3.0",
    surround_mode::surround_4_0, "surround 4.0",
    surround_mode::surround_4_1, "surround 4.1",
    surround_mode::surround_5_0, "surround 5.0",
    surround_mode::surround_5_1, "surround 5.1",
    surround_mode::surround_7_0, "surround 7.0",
    surround_mode::surround_7_1, "surround 7.1",
    surround_mode::surround_9_0, "surround 9.0",
    surround_mode::surround_9_1, "surround 9.1",
    surround_mode::surround_11_0, "surround 11.0",
    surround_mode::surround_11_1, "surround 11.1",
    surround_mode::surround_side_5_0, "surround-side 5.0",
    surround_mode::surround_side_5_1, "surround-side 5.1",
    surround_mode::surround_side_6_0, "surround-side 6.0",
    surround_mode::surround_side_6_1, "surround-side 6.1",
    surround_mode::surround_side_7_0, "surround-side 7.0",
    surround_mode::surround_side_7_1, "surround-side 7.1",
    surround_mode::surround_wide_6_0, "surround-wide 6.0",
    surround_mode::surround_wide_6_1, "surround-wide 6.1",
    surround_mode::surround_wide_7_0, "surround-wide 7.0",
    surround_mode::surround_wide_7_1, "surround-wide 7.1",
    surround_mode::surround_atmos_5_1_4, "surround-atmos 5.1.4",
    surround_mode::surround_atmos_7_1_4, "surround-atmos 7.1.4",
};

export constexpr auto surround_mode_short_names = enum_metadata{
    surround_mode::none, "0.0",
    surround_mode::mono_1_0, "1.0",
    surround_mode::stereo_2_0, "2.0",
    surround_mode::stereo_2_1, "2.1",
    surround_mode::stereo_3_0, "3.0m",
    surround_mode::stereo_3_1, "3.1m",
    surround_mode::quad_4_0, "4.0m",
    surround_mode::quad_side_4_0, "4.0s",
    surround_mode::hexagonal_6_0, "6.0m",
    surround_mode::hexagonal_6_1, "6.1m",
    surround_mode::octagonal_8_0, "8.0m",
    surround_mode::surround_3_0, "3.0",
    surround_mode::surround_4_0, "4.0",
    surround_mode::surround_4_1, "4.1",
    surround_mode::surround_5_0, "5.0",
    surround_mode::surround_5_1, "5.1",
    surround_mode::surround_7_0, "7.0",
    surround_mode::surround_7_1, "7.1",
    surround_mode::surround_9_0, "9.0",
    surround_mode::surround_9_1, "9.1",
    surround_mode::surround_11_0, "11.0",
    surround_mode::surround_11_1, "11.1",
    surround_mode::surround_side_5_0, "5.0s",
    surround_mode::surround_side_5_1, "5.1s",
    surround_mode::surround_side_6_0, "6.0s",
    surround_mode::surround_side_6_1, "6.1s",
    surround_mode::surround_side_7_0, "7.0s",
    surround_mode::surround_side_7_1, "7.1s",
    surround_mode::surround_wide_6_0, "6.0w",
    surround_mode::surround_wide_6_1, "6.1w",
    surround_mode::surround_wide_7_0, "7.0w",
    surround_mode::surround_wide_7_1, "7.1w",
    surround_mode::surround_atmos_5_1_4, "5.1.4",
    surround_mode::surround_atmos_7_1_4, "7.1.4",
};

export constexpr auto surround_mode_speaker_mappings = enum_metadata{
    surround_mode::mono_1_0, speaker_mapping::mono_1_0,
    surround_mode::stereo_2_0, speaker_mapping::stereo_2_0,
    surround_mode::stereo_2_1, speaker_mapping::stereo_2_1,
    surround_mode::stereo_3_0, speaker_mapping::stereo_3_0,
    surround_mode::stereo_3_1, speaker_mapping::stereo_3_1,
    surround_mode::quad_4_0, speaker_mapping::quad_4_0,
    surround_mode::quad_side_4_0, speaker_mapping::quad_side_4_0,
    surround_mode::hexagonal_6_0, speaker_mapping::hexagonal_6_0,
    surround_mode::hexagonal_6_1, speaker_mapping::hexagonal_6_1,
    surround_mode::octagonal_8_0, speaker_mapping::octagonal_8_0,
    surround_mode::surround_3_0, speaker_mapping::surround_3_0,
    surround_mode::surround_4_0, speaker_mapping::surround_4_0,
    surround_mode::surround_4_1, speaker_mapping::surround_4_1,
    surround_mode::surround_5_0, speaker_mapping::surround_5_0,
    surround_mode::surround_5_1, speaker_mapping::surround_5_1,
    surround_mode::surround_7_0, speaker_mapping::surround_7_0,
    surround_mode::surround_7_1, speaker_mapping::surround_7_1,
    surround_mode::surround_9_0, speaker_mapping::surround_9_0,
    surround_mode::surround_9_1, speaker_mapping::surround_9_1,
    surround_mode::surround_11_0, speaker_mapping::surround_11_0,
    surround_mode::surround_11_1, speaker_mapping::surround_11_1,
    surround_mode::surround_side_5_0, speaker_mapping::surround_side_5_0,
    surround_mode::surround_side_5_1, speaker_mapping::surround_side_5_1,
    surround_mode::surround_side_6_0, speaker_mapping::surround_side_6_0,
    surround_mode::surround_side_6_1, speaker_mapping::surround_side_6_1,
    surround_mode::surround_side_7_0, speaker_mapping::surround_side_7_0,
    surround_mode::surround_side_7_1, speaker_mapping::surround_side_7_1,
    surround_mode::surround_wide_6_0, speaker_mapping::surround_wide_6_0,
    surround_mode::surround_wide_6_1, speaker_mapping::surround_wide_6_1,
    surround_mode::surround_wide_7_0, speaker_mapping::surround_wide_7_0,
    surround_mode::surround_wide_7_1, speaker_mapping::surround_wide_7_1,
    surround_mode::surround_atmos_5_1_4, speaker_mapping::surround_atmos_5_1_4,
    surround_mode::surround_atmos_7_1_4, speaker_mapping::surround_atmos_7_1_4
};

// clang-format on

export [[nodiscard]] constexpr surround_mode operator&(surround_mode const& lhs, surround_mode const& rhs) noexcept
{
    return static_cast<surround_mode>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

export [[nodiscard]] constexpr surround_mode operator|(surround_mode const& lhs, surround_mode const& rhs) noexcept
{
    return static_cast<surround_mode>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

export constexpr surround_mode& operator|=(surround_mode& lhs, surround_mode const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

export [[nodiscard]] constexpr bool to_bool(surround_mode const& rhs) noexcept
{
    return to_bool(std::to_underlying(rhs));
}

export [[nodiscard]] constexpr speaker_mapping to_speaker_mapping(surround_mode const& rhs) noexcept
{
    return surround_mode_speaker_mappings[rhs];
}

export [[nodiscard]] generator<surround_mode> enumerate_surround_modes() noexcept
{
    hilet begin = std::to_underlying(surround_mode::mono_1_0);
    hilet end = std::to_underlying(surround_mode::surround_atmos_7_1_4) << 1;

    for (uint64_t i = begin; i != end; i <<= 1) {
        hilet mode = static_cast<surround_mode>(i);
        co_yield mode;
    }
}

export [[nodiscard]] constexpr std::string_view to_string_view_one(surround_mode const& mode) noexcept
{
    return surround_mode_short_names[mode];
}

export [[nodiscard]] constexpr std::string to_string(surround_mode const& mask) noexcept
{
    switch (std::popcount(std::to_underlying(mask))) {
    case 0:
        return std::string{"-"};
    case 1:
        return std::string{to_string_view_one(mask)};
    default:
        {
            hilet begin = std::to_underlying(surround_mode::mono_1_0);
            hilet end = std::to_underlying(surround_mode::surround_atmos_7_1_4) << 1;

            auto r = std::string{};
            for (uint64_t i = begin; i != end; i <<= 1) {
                hilet mode = static_cast<surround_mode>(i);
                if (to_bool(mode & mask)) {
                    if (not r.empty()) {
                        r += ',';
                    }
                    r += to_string_view_one(mode);
                }
            }
            return r;
        }
    }
}

}} // namespace hi::inline v1

// XXX #617 MSVC bug does not handle partial specialization in modules.
export template<>
struct std::formatter<hi::surround_mode, char> : std::formatter<std::string_view, char> {
    auto format(hi::surround_mode const& t, auto& fc) const
    {
        return std::formatter<std::string_view, char>::format(hi::to_string(t), fc);
    }
};
