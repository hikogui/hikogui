// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/hires_utc_clock.hpp"
#include "TTauri/GUI/MouseButtons.hpp"

namespace TTauri::GUI {

struct MouseEvent {
    enum class Type { None, Entered, Exited, Move, Drag, ButtonDown, ButtonUp };

    Type type;

    hires_utc_clock::time_point timePoint;

    //! The current position of the mouse pointer.
    vec position;

    //! The position the last time a button was pressed.
    vec downPosition;

    //! Buttons which has caused this event.
    MouseButtons cause;

    //! Buttons that are pressed/held down.
    MouseButtons down;

    //! Number of clicks from the last button clicked.
    int clickCount;

    MouseEvent() noexcept :
        type(Type::None),
        position(),
        cause(),
        down(),
        clickCount(0)
    {
    }

    static MouseEvent entered(vec position=vec::point(0.0, 0.0)) noexcept {
        MouseEvent event;
        event.position = position;
        event.type = MouseEvent::Type::Entered;
        return event;
    }
    static MouseEvent exited(vec position=vec::point(0.0, 0.0)) noexcept {
        MouseEvent event;
        event.position = position;
        event.type = MouseEvent::Type::Exited;
        return event;
    }
};



}