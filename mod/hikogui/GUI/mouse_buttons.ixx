// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstdint>

export module hikogui_GUI : mouse_buttons;

export namespace hi::inline v1 {

struct mouse_buttons {
    uint8_t left_button : 1;
    uint8_t middle_button : 1;
    uint8_t right_button : 1;
    uint8_t x1_button : 1;
    uint8_t x2_button : 1;

    constexpr mouse_buttons() noexcept :
        left_button(false),
        middle_button(false),
        right_button(false),
        x1_button(false),
        x2_button(false)
    {
    }
};

} // namespace hi::inline v1
