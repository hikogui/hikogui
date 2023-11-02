// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <string_view>
#include <utility>
#include <format>

hi_export_module(hikogui.audio.audio_direction);

hi_export namespace hi { inline namespace v1 {

hi_export enum class audio_direction : unsigned char { none = 0b00, input = 0b01, output = 0b10, bidirectional = 0b11 };

hi_export [[nodiscard]] constexpr audio_direction operator&(audio_direction const& lhs, audio_direction const& rhs) noexcept
{
    return static_cast<audio_direction>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

hi_export [[nodiscard]] constexpr audio_direction operator|(audio_direction const& lhs, audio_direction const& rhs) noexcept
{
    return static_cast<audio_direction>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

hi_export [[nodiscard]] constexpr bool to_bool(audio_direction const &rhs) noexcept
{
    return to_bool(std::to_underlying(rhs));
}

// clang-format off
hi_export constexpr auto audio_direction_metadata = enum_metadata{
    audio_direction::none, "none",
    audio_direction::input, "input",
    audio_direction::output, "output",
    audio_direction::bidirectional, "bidirectional"
};
// clang-format on

}}

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::audio_direction, char> : std::formatter<std::string_view, char> {
    auto format(hi::audio_direction const& t, auto& fc) const
    {
        return std::formatter<std::string_view, char>::format(hi::audio_direction_metadata[t], fc);
    }
};
