// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../cast.hpp"

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

[[nodiscard]] constexpr bool any(audio_direction const &rhs) noexcept
{
    return static_cast<bool>(to_underlying(rhs));
}

}
