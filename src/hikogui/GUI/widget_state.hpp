// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../concurrency/concurrency.hpp"
#include "../theme/theme.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <utility>
#include <concepts>
#include <compare>

hi_export_module(hikogui.GUI : widget_state);

hi_export namespace hi { inline namespace v1 {

/** The mode that the widget is operating at.
 *
 * The enumeration here increases visibility and interactivity at each step
 * and you may use `operator<=>()` to compare modes.
 *
 * @ingroup widget_utilities
 */
enum class widget_mode {
    /** The widget is collapsed.
     *
     * This means:
     *  - The widget has zero size and zero margins.
     *  - The widget does not draw itself or its children.
     *  - The widget will not accept any events.
     */
    collapse = 0,

    /** The widget is invisible.
     *
     * This means:
     *  - The widget has size and margins so that it will reserve space in a container.
     */
    invisible = 1,

    /** The widget is disabled.
     *
     * This means:
     *  - The widget "grayed-out"; drawn with less contrast and saturation.
     */
    disabled = 2,

    /** The widget is in display-only mode.
     *
     * This means:
     *  - The widget is drawn normally.
     */
    display = 3,

    /** The widget is selectable.
     *
     * This means:
     *  - The widget or its contents such as text may be selected.
     *  - The widget or its contents may be dragged by the mouse.
     */
    select = 4,

    /** A widget is partially enabled.
     *
     * This means:
     *  - The widget will accept keyboard focus.
     *  - A widget has an extra mode where it limits the amount of control.
     *    such as a text-widget which has a mode where only a single line
     *    can be edited.
     */
    partial = 5,

    /** The widget is fully enabled.
     *
     * This means:
     *  - The widget will accept keyboard focus.
     *  - The widget's state is controllable.
     */
    enabled = 6,
};

enum class widget_value {
    off = 0,
    on = 1,
    other = 2,
};

enum class widget_phase {
    inactive = 0,
    normal = 1,
    hover = 2,
    pressed = 3,
};

constexpr auto widget_state_mode_shift = 0;
constexpr auto widget_state_value_shift = 5;

/** The state the widget is in.
 * 
 * The numeric value of the state is used as an index into theme-values
 * to select the appropriate visual style.
 */
class widget_state {
public:
    constexpr widget_state(widget_state const &) noexcept = default;
    constexpr widget_state(widget_state &&) noexcept = default;
    constexpr widget_state &operator=(widget_state const &) noexcept = default;
    constexpr widget_state &operator=(widget_state &&) noexcept = default;
    
    constexpr widget_state() noexcept = default;

    /** Start of the iteration of all possible widget states.
     */
    constexpr static widget_state begin() noexcept
    {
        auto r = widget_state{};
        r.set_mode(widget_mode::collapse);
        r.set_layer(0);
        r.set_value(widget_value::off);
        r.set_pressed(false);
        r.set_hover(false);
        r.set_active(false);
        r.set_focus(false);
        return r;
    }

    /** End of the iteration of all possible widget states.
     */
    constexpr static widget_state end() noexcept
    {
        auto r = begin();
        r._end = true;
        return r;
    }

    /** The number if possible widget states.
     */
    constexpr static size_t size() noexcept
    {
        return static_cast<size_t>(end());
    }

    /** Get the mode of a widget.
     *
     * @see widget_mode
     */
    [[nodiscard]] constexpr widget_mode mode() const noexcept
    {
        return static_cast<widget_mode>(_mode);
    }

    /** Set the mode of a widget.
     *
     * @see widget_mode
     */
    constexpr widget_state &set_mode(widget_mode mode) noexcept
    {
        _mode = static_cast<uint16_t>(mode);
        return *this;
    }

    /** Get the layer of a widget.
     *
     * The layer between 0 and 3 is used to determine how to visual
     * distinct widget at different nesting levels.
     */
    [[nodiscard]] constexpr size_t layer() const noexcept
    {
        return _layer;
    }

    /** Set the layer of the widget
     * 
     * @param layer The layer is modulo 4.
     */
    constexpr widget_state &set_layer(size_t layer) noexcept
    {
        _layer = static_cast<uint16_t>(layer % 4);
        return *this;
    }

    /** Get the value of the widget.
     * 
     * @see widget_value
     */
    [[nodiscard]] constexpr widget_value value() const noexcept
    {
        return static_cast<widget_value>(_value);
    }

    /** Set the value of the widget.
     * 
     * @see widget_value
     */
    constexpr widget_state &set_value(widget_value value) noexcept
    {
        _value = static_cast<uint16_t>(value);
        return *this;
    }

    /** Get the phase of the widget.
     * 
     * @see widget_phase
     */
    [[nodiscard]] constexpr widget_phase phase() const noexcept
    {
        if (_pressed) {
            return widget_phase::pressed;
        } else if (_hover) {
            return widget_phase::hover;
        } else if (_active) {
            return widget_phase::normal;
        } else {
            return widget_phase::inactive;
        }
    }

    [[nodiscard]] constexpr bool pressed() const noexcept
    {
        return _pressed;
    }

    /** Set if the mouse/finger presses the widget.
     * 
     * @see widget_phase
     */
    constexpr widget_state &set_pressed(bool pressed) noexcept
    {
        _pressed = static_cast<bool>(pressed);
        return *this;
    }

    [[nodiscard]] constexpr bool hover() const noexcept
    {
        return _hover;
    }

    /** Set if the mouse hovers over the widget.
     * 
     * @see widget_phase
     */
    constexpr widget_state &set_hover(bool hover) noexcept
    {
        _hover = static_cast<bool>(hover);
        return *this;
    }

    [[nodiscard]] constexpr bool active() const noexcept
    {
        return _active;
    }

    /** Set if the window is active widget.
     * 
     * @see widget_phase
     */
    constexpr widget_state &set_active(bool active) noexcept
    {
        _active = static_cast<bool>(active);
        return *this;
    }

    /** Get if the window has keyboard focus.
     */
    [[nodiscard]] constexpr bool focus() const noexcept
    {
        return static_cast<bool>(_focus);
    }

    /** Set if the window has keyboard focus.
     */
    constexpr widget_state &set_focus(bool focus) noexcept
    {
        _focus = static_cast<uint16_t>(focus);
        return *this;
    }

    constexpr style_pseudo_class pseudo_class() const noexcept
    {
        auto r = style_pseudo_class{};

        if (pressed()) {
            r |= style_pseudo_class::active;
        } else if (hover()) {
            r |= style_pseudo_class::hover;
        } else if (active()) {
            r |= style_pseudo_class::enabled;
        } else {
            r |= style_pseudo_class::disabled;
        }

        if (focus()) {
            r |= style_pseudo_class::focus;
        }

        if (value() != widget_value::off) {
            r |= style_pseudo_class::checked;
        }
        
        return r;
    }

    /** Get the numeric value of the window state.
     * 
     * The numeric value is used for indexing into theme tables
     * for quick lookup of, for example, the background color of
     * the widget in a certain state.
     */
    template<std::integral T>
    constexpr explicit operator T() const noexcept
        requires(sizeof(T) >= 2)
    {
        auto r = T{0};

        r += static_cast<T>(_end);

        r *= 2;
        r += static_cast<T>(focus());

        r *= 4;
        r += static_cast<T>(phase());

        r *= 3;
        r += static_cast<T>(value());

        r *= 4;
        r += static_cast<T>(layer());

        r *= 7;
        r += static_cast<T>(mode());
        return r;
    }

    [[nodiscard]] constexpr friend bool operator==(widget_state const &lhs, widget_state const &rhs) noexcept
    {
        return static_cast<uint16_t>(lhs) == static_cast<uint16_t>(rhs);
    }

    /** Increment the widget-state.
     * 
     * This is used to iterate over each unique widget_state, to fill in
     * the theme tables.
     */
    constexpr widget_state& operator++() noexcept
    {
        if (mode() < widget_mode::enabled) {
            set_mode(static_cast<widget_mode>(std::to_underlying(mode()) + 1));
            return *this;
        }
        set_mode(widget_mode::collapse);

        if (layer() < 3) {
            set_layer(layer() + 1);
            return *this;
        }
        set_layer(0);

        if (value() < widget_value::other) {
            set_value(static_cast<widget_value>(std::to_underlying(value()) + 1));
            return *this;
        }
        set_value(widget_value::off);

        switch (phase()) {
        case widget_phase::inactive:
            set_pressed(false);
            set_hover(false);
            set_active(true);
            return *this;
        case widget_phase::normal:
            set_pressed(false);
            set_hover(true);
            set_active(true);
            return *this;
        case widget_phase::hover:
            set_pressed(true);
            set_hover(true);
            set_active(true);
            return *this;
        case widget_phase::pressed:
            set_pressed(false);
            set_hover(false);
            set_active(false);
            break;
        default:
            hi_no_default();
        }

        if (focus() == false) {
            set_focus(true);
            return *this;
        }
        set_focus(false);

        _end = true;
        return *this;
    }

private:
    /** hi::widget_mode.
     */
    uint16_t _mode : 3 = static_cast<uint16_t>(widget_mode::enabled);

    /** Widget depth layer used for visually separating layers of nest widget.
     */
    uint16_t _layer : 2 = 0;

    /** hi::widget_value.
     */
    uint16_t _value : 2 = static_cast<uint16_t>(widget_value::off);

    /** The window is active.
     */
    uint16_t _active: 1 = 1;

    /** The mouse hovers over the widget.
    */
    uint16_t _hover : 1 = 0;

    /** The mouse clicked the widget.
    */
    uint16_t _pressed : 1 = 0;

    /** The widget has keyboard focus.
    */
    uint16_t _focus : 1 = 0;

    /** Marker for end of iteration.
     */
    uint16_t _end : 1 = 0;
};

}}
