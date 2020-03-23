// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>

namespace TTauri::GUI {

struct MouseButtons {
    uint8_t leftButton:1;
    uint8_t middleButton:1;
    uint8_t rightButton:1;
    uint8_t x1Button:1;
    uint8_t x2Button:1;
    uint8_t controlKey:1;
    uint8_t shiftKey:1;

    MouseButtons() noexcept :
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

}