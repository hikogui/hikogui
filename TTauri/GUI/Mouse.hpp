// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <glm/glm.hpp>

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
    MoveArea
};

enum class Cursor {
    None,
    Default,
    Clickable
};

struct MouseButtons {
    bool leftButton:1;
    bool middleButton:1;
    bool rightButton:1;
    bool x1Button:1;
    bool x2Button:1;
    bool controlKey:1;
    bool shiftKey:1;

    MouseButtons() :
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
    glm::vec2 position;

    //! Buttons which has caused this event.
    MouseButtons cause;

    //! Buttons that are pressed/held down.
    MouseButtons down;

    MouseEvent() :
        type(Type::None),
        position(),
        cause(),
        down()
    {
    }
};

inline MouseEvent ExitedMouseEvent() {
    MouseEvent event;
    event.type = MouseEvent::Type::Exited;
    return event;
}

}