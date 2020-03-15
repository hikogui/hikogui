// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"

namespace TTauri::GUI {

enum class HitBox {
    NoWhereInteresting,
    BottomResizeBorder,
    TopResizeBorder,
    LeftResizeBorder,
    RightResizeBorder,
    BottomLeftResizeCorner,
    BottomRightResizeCorner,
    TopLeftResizeCorner,
    TopRightResizeCorner,
    ApplicationIcon,
    MoveArea
};

enum class Cursor {
    None,
    Default,
    Clickable
};

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

struct MouseEvent {
    enum class Type { None, Exited, Move, ButtonDown, ButtonUp, ButtonDoubleClick };

    Type type;
    vec position;

    //! Buttons which has caused this event.
    MouseButtons cause;

    //! Buttons that are pressed/held down.
    MouseButtons down;

    MouseEvent() noexcept :
        type(Type::None),
        position(),
        cause(),
        down()
    {
    }
};

inline MouseEvent ExitedMouseEvent(vec position=vec::point(0.0, 0.0)) noexcept {
    MouseEvent event;
    event.position = position;
    event.type = MouseEvent::Type::Exited;
    return event;
}

}