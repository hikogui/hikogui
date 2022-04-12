// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mouse_buttons.hpp"
#include "../geometry/point.hpp"
#include "../geometry/vector.hpp"
#include "../geometry/transform.hpp"
#include <chrono>

namespace tt::inline v1 {

struct mouse_event {
    enum class Type { None, Entered, Exited, Move, Drag, ButtonDown, ButtonUp, Wheel };

    Type type;

    std::chrono::utc_time<std::chrono::nanoseconds> timePoint;

    //! The current position of the mouse pointer.
    point2 position;

    //! The position the last time a button was pressed.
    point2 downPosition;

    //! Change in wheel rotation, in pixels.
    vector2 wheelDelta;

    //! Buttons which has caused this event.
    mouse_buttons cause;

    //! Buttons that are pressed/held down.
    mouse_buttons down;

    //! Number of clicks from the last button clicked.
    int clickCount;

    mouse_event() noexcept : type(Type::None), position(), cause(), down(), clickCount(0) {}

    [[nodiscard]] bool empty() const noexcept
    {
        return type == Type::None;
    }

    operator bool() const noexcept
    {
        return not empty();
    }

    static mouse_event entered(point2 position = {}) noexcept
    {
        mouse_event event;
        event.position = position;
        event.type = mouse_event::Type::Entered;
        return event;
    }

    static mouse_event exited() noexcept
    {
        // Position far away from the left/bottom corner, but where even
        // after translations will not cause the position to be infinite.
        constexpr float far_ = std::numeric_limits<float>::max() * -0.5f;

        mouse_event event;
        event.position = point2{far_, far_};
        event.type = mouse_event::Type::Exited;
        return event;
    }

    /** Check if this event is for a left-button-up event while the mouse pointer is in the given area.
     */
    [[nodiscard]] bool is_left_button_up(aarectangle active_area) const noexcept
    {
        return type == Type::ButtonUp and cause.leftButton and active_area.contains(position);
    }

    /** Get the location of the mouse relative to the start of a drag.
     */
    [[nodiscard]] vector2 delta() const noexcept
    {
        return type == Type::Drag ? position - downPosition : vector2{};
    }

    [[nodiscard]] friend mouse_event operator*(geo::transformer auto const &transform, mouse_event const &rhs) noexcept
    {
        auto r = rhs;
        r.position = point2{transform * rhs.position};
        r.downPosition = point2{transform * rhs.downPosition};
        r.wheelDelta = vector2{transform * rhs.wheelDelta};
        return r;
    }

    friend std::string to_string(mouse_event const &rhs) noexcept
    {
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

        return std::format("<mouse {} {}>", type_s, rhs.position);
    }

    friend std::ostream &operator<<(std::ostream &lhs, mouse_event const &rhs)
    {
        return lhs << to_string(rhs);
    }
};

} // namespace tt::inline v1
