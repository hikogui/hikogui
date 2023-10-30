// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <utility>

hi_export_module(hikogui.GUI : keyboard_state);

hi_export namespace hi::inline v1 {

enum class keyboard_state : uint8_t {
    idle = 0x00,
    caps_lock = 0x01,
    scroll_lock = 0x02,
    num_lock = 0x04,
};

[[nodiscard]] constexpr keyboard_state operator|(keyboard_state const& lhs, keyboard_state const& rhs) noexcept
{
    return static_cast<keyboard_state>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr keyboard_state operator&(keyboard_state const& lhs, keyboard_state const& rhs) noexcept
{
    return static_cast<keyboard_state>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

constexpr keyboard_state& operator|=(keyboard_state& lhs, keyboard_state const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

bool operator>=(keyboard_state const& lhs, keyboard_state const& rhs) = delete;

[[nodiscard]] constexpr bool to_bool(keyboard_state const& rhs) noexcept
{
    return to_bool(std::to_underlying(rhs));
}

}
