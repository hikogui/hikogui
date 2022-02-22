// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/widget.hpp"
#include "ttauri/crt.hpp"
#include "ttauri/log.hpp"

// Every widget must inherit from tt::widget.
class command_widget : public tt::widget {
public:
    // Using an observable allows reading, writing and monitoring of the value outside of the widget.
    tt::observable<bool> value;

    // Every constructor of a widget starts with a `window` and `parent` argument.
    // In most cases these are automatically filled in when calling a container widget's `make_widget()` function.
    command_widget(tt::gui_window &window, tt::widget *parent) noexcept : widget(window, parent)
    {
        // To visually show the change in value the widget needs to be redrawn.
        value.subscribe(_redraw_callback);
    }

    // The set_constraints() function is called when the window is first initialized,
    // or when a widget wants to change its constraints.
    tt::widget_constraints const &set_constraints() noexcept override
    {
        // Reset _layout so that the set_layout() calculations will be triggered.
        _layout = {};

        // Set the minimum, preferred, maximum sizes and the margin around the widget.
        return _constraints = {{100.0f, 20.0f}, {200.0f, 20.0f}, {300.0f, 50.0f}, theme().margin};
    }

    // The `set_layout()` function is called when the window has resized, or when
    // a widget wants to change the internal layout.
    //
    // NOTE: The size of the layout may be larger than the maximum constraints of this widget.
    void set_layout(tt::widget_layout const &layout) noexcept override
    {
        // Update the `_layout` with the new context.
        if (compare_store(_layout, layout)) {}
    }

    // It is common to override the context sensitive colors of the default widget.
    // In this case the background color is 'teal' when the value of the widget is true.
    [[nodiscard]] tt::color background_color() const noexcept override
    {
        return value ? theme().color(tt::theme_color::green) : widget::background_color();
    }

    // The `draw()` function is called when all or part of the window requires redrawing.
    // This may happen when showing the window for the first time, when the operating-system
    // requests a (partial) redraw, or when a widget requests a redraw of itself.
    void draw(tt::draw_context const &context) noexcept override
    {
        // We only need to draw the widget when it is visible and when the visible area of
        // the widget overlaps with the scissor-rectangle (partial redraw) of the drawing context.
        if (visible and overlaps(context, layout())) {
            // When drawing this box we use the widget's background_color() and focus_color().
            // These colors are context sensitive; for example focus_color() checks if the widget is enabled,
            // has keyboard focus and the window is active.
            context.draw_box(
                _layout,
                _layout.rectangle(),
                background_color(),
                focus_color(),
                theme().border_width,
                tt::border_side::inside,
                theme().rounding_radius);
        }
    }

    // Override this function when your widget needs to be controllable by keyboard interaction.
    [[nodiscard]] bool accepts_keyboard_focus(tt::keyboard_focus_group group) const noexcept override
    {
        // This widget will react to "normal" tab/shift-tab keys and mouse clicks to focus the widget.
        return enabled and any(group & tt::keyboard_focus_group::normal);
    }

    // Override this function when your widget needs to be controllable by mouse interaction.
    [[nodiscard]] tt::hitbox hitbox_test(tt::point3 position) const noexcept override
    {
        // Check if the (mouse) position is within the visual-area of the widget.
        // The hit_rectangle is the _layout.rectangle() intersected with the _layout.clipping_rectangle.
        if (visible and enabled and layout().contains(position)) {
            // The `this` argument allows the gui_window to forward mouse events to handle_event(mouse) of this widget.
            // The `position` argument is used to handle widgets that are visually overlapping, widgets with higher elevation
            // get priority. When this widget is enabled it should show a button-cursor, otherwise just the normal arrow.
            return {this, position, enabled ? tt::hitbox::Type::Button : tt::hitbox::Type::Default};

        } else {
            return {};
        }
    }

    // Override the handle_event(command) to handle high level commands.
    [[nodiscard]] bool handle_event(tt::command command) noexcept override
    {
        if (enabled and command == tt::command::gui_activate) {
            // Handle activate, by default the "spacebar" causes this command.
            value = not value;
            return true;

        } else if (enabled and command == tt::command::gui_enter) {
            // Handle the enter command, this will activate then set the keyboard focus to the next normal widget.
            value = not value;
            window.update_keyboard_target(tt::keyboard_focus_group::normal, tt::keyboard_focus_direction::forward);
            return true;
        }

        // The default handle_event() will handle hovering and auto-scrolling.
        return widget::handle_event(command);
    }

    // Override the mouse event to handle the left click.
    [[nodiscard]] bool handle_event(tt::mouse_event const &event) noexcept override
    {
        if (enabled and event.is_left_button_up(_layout.rectangle())) {
            // Forward the left click as a gui_activate command.
            return handle_event(tt::command::gui_activate);
        }

        // The default handle_event() doesn't do anything, but should still be called.
        return widget::handle_event(event);
    }

    bool handle_event(tt::keyboard_event const &event) noexcept
    {
        if (enabled) {
            if (event.type == tt::keyboard_event::Type::grapheme) {
                tt_log_error("User typed the letter U+{:x}.", static_cast<uint32_t>(get<0>(event.grapheme)));
                return true;
            }
        }
        return false;
    }
};

int tt_main(int argc, char *argv[])
{
    auto gui = tt::gui_system::make_unique();
    auto &window = gui->make_window(tt::l10n("Custom Widget Command"));
    window.content().make_widget<command_widget>("A1");
    window.content().make_widget<command_widget>("A2");
    return gui->loop();
}
