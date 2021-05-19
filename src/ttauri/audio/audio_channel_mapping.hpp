// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../architecture.hpp"
#include <string>
#include <iostream>

namespace tt {

enum class audio_channel_mapping : uint32_t {
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
    top_back_right = 0x2'0000
};

[[nodiscard]] inline bool to_bool(audio_channel_mapping const &rhs) noexcept
{
    return static_cast<bool>(static_cast<uint32_t>(rhs));
}

[[nodiscard]] inline audio_channel_mapping operator|(audio_channel_mapping const &lhs, audio_channel_mapping const &rhs) noexcept
{
    return static_cast<audio_channel_mapping>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

[[nodiscard]] inline audio_channel_mapping operator&(audio_channel_mapping const &lhs, audio_channel_mapping const &rhs) noexcept
{
    return static_cast<audio_channel_mapping>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

inline audio_channel_mapping &operator|=(audio_channel_mapping &lhs, audio_channel_mapping const &rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
[[nodiscard]] audio_channel_mapping audio_channel_mapping_from_win32(uint32_t from) noexcept;
[[nodiscard]] uint32_t audio_channel_mapping_to_win32(audio_channel_mapping from) noexcept;
#endif

[[nodiscard]] std::string to_string(audio_channel_mapping rhs) noexcept;

inline std::ostream &operator<<(std::ostream &lhs, audio_channel_mapping rhs)
{
    return lhs << to_string(rhs);
}

}