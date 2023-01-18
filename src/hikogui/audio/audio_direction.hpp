// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"

namespace hi::inline v1 {

enum class audio_direction : unsigned char { none = 0b00, input = 0b01, output = 0b10, bidirectional = 0b11 };

[[nodiscard]] constexpr audio_direction operator&(audio_direction const& lhs, audio_direction const& rhs) noexcept
{
    return static_cast<audio_direction>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr audio_direction operator|(audio_direction const& lhs, audio_direction const& rhs) noexcept
{
    return static_cast<audio_direction>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr bool to_bool(audio_direction const &rhs) noexcept
{
    return to_bool(to_underlying(rhs));
}

// clang-format off
constexpr auto audio_direction_metadata = enum_metadata{
    audio_direction::none, "none",
    audio_direction::input, "input",
    audio_direction::output, "output",
    audio_direction::bidirectional, "bidirectional"
};
// clang-format on

}

template<typename CharT>
struct std::formatter<hi::audio_direction, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::audio_direction const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(hi::audio_direction_metadata[t], fc);
    }
};
