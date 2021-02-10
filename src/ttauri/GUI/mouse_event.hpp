// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mouse_buttons.hpp"
#include "../numeric_array.hpp"
#include "../hires_utc_clock.hpp"

namespace tt {

struct mouse_event {
    enum class Type { None, Entered, Exited, Move, Drag, ButtonDown, ButtonUp, Wheel };

    Type type;

    hires_utc_clock::time_point timePoint;

    //! The current position of the mouse pointer.
    f32x4 position;

    //! The position the last time a button was pressed.
    f32x4 downPosition;

    //! Change in wheel rotation, in pixels.
    f32x4 wheelDelta;

    //! Buttons which has caused this event.
    mouse_buttons cause;

    //! Buttons that are pressed/held down.
    mouse_buttons down;

    //! Number of clicks from the last button clicked.
    int clickCount;

    mouse_event() noexcept :
        type(Type::None),
        position(),
        cause(),
        down(),
        clickCount(0)
    {
    }

    /** Get the location of the mouse relative to the start of a drag.
     */
    [[nodiscard]] f32x4 delta() const noexcept {
        return type == Type::Drag ? position - downPosition : f32x4{};
    }

    static mouse_event entered(f32x4 position=f32x4::point({0.0f, 0.0f})) noexcept {
        mouse_event event;
        event.position = position;
        event.type = mouse_event::Type::Entered;
        return event;
    }

    static mouse_event exited() noexcept {
        // Position far away from the left/bottom corner, but where even
        // after translations will not cause the position to be infinite.
        constexpr float far_ = std::numeric_limits<float>::max() * -0.5f;

        mouse_event event;
        event.position = f32x4{far_, far_};
        event.type = mouse_event::Type::Exited;
        return event;
    }

    friend std::string to_string(mouse_event const &rhs) noexcept {
        char const *type_s;
        switch (rhs.type) {
        using enum mouse_event::Type;
        case None: type_s = "none"; break;
        case Entered: type_s = "entered"; break;
        case Exited: type_s = "exited"; break;
        case Move: type_s = "move"; break;
        case Drag: type_s = "drag"; break;
        case ButtonDown: type_s = "down"; break;
        case ButtonUp: type_s = "up"; break;
        case Wheel: type_s = "wheel"; break;
        default: tt_no_default();
        }

        return fmt::format("<mouse {} {}>", type_s, rhs.position);
    }

    friend std::ostream &operator<<(std::ostream &lhs, mouse_event const &rhs) {
        return lhs << to_string(rhs);
    }
};



}
