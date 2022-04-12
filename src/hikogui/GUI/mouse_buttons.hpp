// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

namespace hi::inline v1 {

struct mouse_buttons {
    uint8_t leftButton : 1;
    uint8_t middleButton : 1;
    uint8_t rightButton : 1;
    uint8_t x1Button : 1;
    uint8_t x2Button : 1;
    uint8_t controlKey : 1;
    uint8_t shiftKey : 1;

    mouse_buttons() noexcept :
        leftButton(false),
        middleButton(false),
        rightButton(false),
        x1Button(false),
        x2Button(false),
        controlKey(false),
        shiftKey(false)
    {
    }
};

} // namespace hi::inline v1
