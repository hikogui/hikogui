// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_event_type.hpp"
#include "keyboard_virtual_key.hpp"
#include "keyboard_state.hpp"
#include "keyboard_modifiers.hpp"
#include "mouse_buttons.hpp"
#include "../unicode/grapheme.hpp"
#include "../geometry/vector.hpp"
#include "../geometry/point.hpp"
#include "../geometry/transform.hpp"
#include "../chrono.hpp"
#include <chrono>

namespace hi::inline v1 {

struct mouse_event_data {
    /** The current position of the mouse pointer.
     *
     * @note The event system will convert these in widget-local coordinates.
     */
    point2 position = {};

    /** The position the last time a button was pressed.
     *
     * This can be used as the position at the start of a drag event.
     *
     * @note The event system will convert these in widget-local coordinates.
     */
    point2 down_position = {};

    /** Change in wheel rotation, in points (pt).
     *
     * Some mice have two dimensional mouse wheels.
     */
    vector2 wheel_delta = {};

    /** Buttons which have caused this event.
     */
    mouse_buttons cause = {};

    /** Buttons that are also held down.
     */
    mouse_buttons down = {};

    /** Number of clicks from the last button clicked.
     */
    uint8_t click_count = 0;
};

/** A user interface event.
 */
class gui_event {
public:
    /** The time when the event was created.
     */
    utc_nanoseconds time_point;

    /** Keyboard modifiers: shift, ctrl, alt, etc.
     *
     * This may be used for in combination with both keyboard and mouse events.
     */
    keyboard_modifiers keyboard_modifiers;

    /** State of the keyboard; caps-lock, num-lock, scroll-lock.
     */
    keyboard_state keyboard_state;

    constexpr gui_event(
        gui_event_type type,
        utc_nanoseconds time_point,
        hi::keyboard_modifiers keyboard_modifiers,
        hi::keyboard_state keyboard_state) noexcept :
        time_point(time_point), keyboard_modifiers(keyboard_modifiers), keyboard_state(keyboard_state)
    {
        set_type(type);
    }

    constexpr gui_event() noexcept :
        gui_event(gui_event_type::none, utc_nanoseconds{}, keyboard_modifiers::none, keyboard_state::idle)
    {
    }

    gui_event(gui_event_type type) noexcept :
        gui_event(type, std::chrono::utc_clock::now(), keyboard_modifiers::none, keyboard_state::idle)
    {
    }

    gui_event(gui_event_type type, hi::grapheme grapheme) noexcept :
        gui_event(type, std::chrono::utc_clock::now(), keyboard_modifiers::none, keyboard_state::idle)
    {
        hi_axiom(is_grapheme_event());
        this->grapheme() = grapheme;
    }

    gui_event(
        gui_event_type type,
        keyboard_virtual_key key,
        hi::keyboard_modifiers keyboard_modifiers = keyboard_modifiers::none,
        hi::keyboard_state keyboard_state = keyboard_state::idle) noexcept :
        gui_event(type, std::chrono::utc_clock::now(), keyboard_modifiers, keyboard_state)
    {
        hi_axiom(is_keyboard_event());
        this->key() = key;
    }

    constexpr gui_event(gui_event const&) noexcept = default;
    constexpr gui_event(gui_event&&) noexcept = default;
    constexpr gui_event& operator=(gui_event const&) noexcept = default;
    constexpr gui_event& operator=(gui_event&&) noexcept = default;

    [[nodiscard]] static gui_event make_mouse_enter(point2 position) noexcept
    {
        auto r = gui_event{gui_event_type::mouse_enter};
        r.mouse().position = position;
        return r;
    }

    [[nodiscard]] constexpr gui_event_type type() const noexcept
    {
        return _type;
    }

    [[nodiscard]] constexpr void set_type(gui_event_type type) noexcept
    {
        hilet was_mouse_event = is_mouse_event();
        hilet was_grapheme_event = is_grapheme_event();
        hilet was_keyboard_event = is_keyboard_event();

        _type = type;
        if (is_mouse_event() and not was_mouse_event) {
            _mouse = {};

        } else if (is_grapheme_event() and not was_grapheme_event) {
            _grapheme = hi::grapheme{};

        } else if (is_keyboard_event() and not was_keyboard_event) {
            _key = {};
        }
    }

    [[nodiscard]] mouse_event_data& mouse() noexcept
    {
        hi_axiom(is_mouse_event());
        return _mouse;
    }

    [[nodiscard]] mouse_event_data const& mouse() const noexcept
    {
        hi_axiom(is_mouse_event());
        return _mouse;
    }

    [[nodiscard]] keyboard_virtual_key& key() noexcept
    {
        hi_axiom(is_keyboard_event());
        return _key;
    }

    [[nodiscard]] keyboard_virtual_key const& key() const noexcept
    {
        hi_axiom(is_keyboard_event());
        return _key;
    }

    [[nodiscard]] hi::grapheme& grapheme() noexcept
    {
        hi_axiom(is_grapheme_event());
        return _grapheme;
    }

    [[nodiscard]] hi::grapheme const& grapheme() const noexcept
    {
        hi_axiom(is_grapheme_event());
        return _grapheme;
    }

    [[nodiscard]] constexpr bool operator==(gui_event_type event_type) const noexcept
    {
        return type() == event_type;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return type() == gui_event_type::none;
    }

    constexpr operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr bool is_mouse_event() const noexcept
    {
        using enum gui_event_type;
        if (type() == mouse_move or type() == mouse_drag or type() == mouse_down or type() == mouse_up or type() == mouse_wheel or
            type() == mouse_exit or type() == mouse_enter or type() == mouse_exit_window) {
            hi_axiom(not is_grapheme_event());
            hi_axiom(not is_keyboard_event());
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] constexpr bool is_grapheme_event() const noexcept
    {
        using enum gui_event_type;
        if (type() == keyboard_grapheme or type() == keyboard_partial_grapheme) {
            hi_axiom(not is_mouse_event());
            hi_axiom(not is_keyboard_event());
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] constexpr bool is_keyboard_event() const noexcept
    {
        using enum gui_event_type;
        if (type() == keyboard_down) {
            hi_axiom(not is_mouse_event());
            hi_axiom(not is_grapheme_event());
            return true;
        } else {
            return false;
        }
    }

    /** Check if this event is for a left-button-up event while the mouse pointer is in the given area.
     */
    [[nodiscard]] constexpr bool is_left_button_up(aarectangle active_area) const noexcept
    {
        using enum gui_event_type;
        return type() == mouse_up and mouse().cause.left_button and active_area.contains(mouse().position);
    }

    /** Get the location of the mouse relative to the start of a drag.
     */
    [[nodiscard]] constexpr vector2 drag_delta() const noexcept
    {
        using enum gui_event_type;
        return type() == mouse_drag ? mouse().position - mouse().down_position : vector2{};
    }

    [[nodiscard]] constexpr friend gui_event operator*(geo::transformer auto const& transform, gui_event const& rhs) noexcept
    {
        auto r = rhs;
        if (rhs.is_mouse_event()) {
            r.mouse().position = point2{transform * rhs.mouse().position};
            r.mouse().down_position = point2{transform * rhs.mouse().down_position};
            r.mouse().wheel_delta = vector2{transform * rhs.mouse().wheel_delta};
        }
        return r;
    }

private:
    gui_event_type _type;

    union {
        mouse_event_data _mouse;
        keyboard_virtual_key _key;
        hi::grapheme _grapheme;
    };
};

} // namespace hi::inline v1
