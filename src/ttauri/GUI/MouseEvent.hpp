// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/vec.hpp"
#include "ttauri/hires_utc_clock.hpp"
#include "TTauri/GUI/MouseButtons.hpp"

namespace tt {

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

    static MouseEvent entered(vec position=vec::point(0.0f, 0.0f)) noexcept {
        MouseEvent event;
        event.position = position;
        event.type = MouseEvent::Type::Entered;
        return event;
    }
    static MouseEvent exited() noexcept {
        // Position far away from the left/bottom corner, but where even
        // after translations will not cause the position to be infinite.
        constexpr float far_ = std::numeric_limits<float>::max() * -0.5f;

        MouseEvent event;
        event.position = vec{far_, far_};
        event.type = MouseEvent::Type::Exited;
        return event;
    }

    friend std::string to_string(MouseEvent const &rhs) noexcept {
        char const *type_s;
        switch (rhs.type) {
        case MouseEvent::Type::None: type_s = "none"; break;
        case MouseEvent::Type::Entered: type_s = "entered"; break;
        case MouseEvent::Type::Exited: type_s = "exited"; break;
        case MouseEvent::Type::Move: type_s = "move"; break;
        case MouseEvent::Type::Drag: type_s = "drag"; break;
        case MouseEvent::Type::ButtonDown: type_s = "down"; break;
        case MouseEvent::Type::ButtonUp: type_s = "up"; break;
        default: tt_no_default;
        }

        return fmt::format("<mouse {} {}>", type_s, rhs.position);
    }

    friend std::ostream &operator<<(std::ostream &lhs, MouseEvent const &rhs) {
        return lhs << to_string(rhs);
    }
};



}