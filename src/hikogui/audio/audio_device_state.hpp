// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include <format>
#include <string_view>

namespace hi::inline v1 {

enum class audio_device_state { uninitialized, active, disabled, not_present, unplugged };

// clang-format off
constexpr auto audio_device_state_metadata = enum_metadata{
    audio_device_state::uninitialized, "uninitialized",
    audio_device_state::active, "active",
    audio_device_state::disabled, "disabled",
    audio_device_state::not_present, "not_present",
    audio_device_state::unplugged, "unplugged",
};
// clang-format on

[[nodiscard]] constexpr std::string_view to_string(audio_device_state const& rhs) noexcept
{
    return audio_device_state_metadata[rhs];
}

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::audio_device_state, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::audio_device_state const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(hi::audio_device_state_metadata[t], fc);
    }
};
