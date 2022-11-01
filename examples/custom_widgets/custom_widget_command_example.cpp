// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/widgets/widget.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/log.hpp"
#include "hikogui/loop.hpp"

// Every widget must inherit from hi::widget.
class command_widget : public hi::widget {
public:
    // Using an observer allows reading, writing and monitoring of the value outside of the widget.
    hi::observer<bool> value;

    // Every constructor of a widget starts with a `window` and `parent` argument.
    // In most cases these are automatically filled in when calling a container widget's `make_widget()` function.
    command_widget(hi::widget *parent) noexcept : hi::widget(parent)
    {
        // To visually show the change in value the widget needs to be redrawn.
        _value_cbt = value.subscribe([&](auto...) {
            request_redraw();
        });
    }

    // The set_constraints() function is called when the window is first initialized,
    // or when a widget wants to change its constraints.
    hi::widget_constraints const& set_constraints(hi::set_constraints_context const& context) noexcept override
    {
        // Reset _layout so that the set_layout() calculations will be triggered.
        _layout = {};

        // Set the minimum, preferred, maximum sizes and the margin around the widget.
        return _constraints = {{100.0f, 20.0f}, {200.0f, 20.0f}, {300.0f, 50.0f}, context.theme->margin};
    }

    // The `set_layout()` function is called when the window has resized, or when
    // a widget wants to change the internal layout.
    //
    // NOTE: The size of the layout may be larger than the maximum constraints of this widget.
    void set_layout(hi::widget_layout const& context) noexcept override
    {
        // Update the `_layout` with the new context.
        if (compare_store(_layout, context)) {}
    }

    // It is common to override the context sensitive colors of the default widget.
    // In this case the background color is 'teal' when the value of the widget is true.
    [[nodiscard]] hi::color background_color() const noexcept override
    {
        return *value ? layout().theme->color(hi::semantic_color::green) : widget::background_color();
    }

    // The `draw()` function is called when all or part of the window requires redrawing.
    // This may happen when showing the window for the first time, when the operating-system
    // requests a (partial) redraw, or when a widget requests a redraw of itself.
    void draw(hi::draw_context const& context) noexcept override
    {
        // We only need to draw the widget when it is visible and when the visible area of
        // the widget overlaps with the scissor-rectangle (partial redraw) of the drawing context.
        if (*mode > hi::widget_mode::invisible and overlaps(context, layout())) {
            // When drawing this box we use the widget's background_color() and focus_color().
            // These colors are context sensitive; for example focus_color() checks if the widget is enabled,
            // has keyboard focus and the window is active.
            context.draw_box(
                _layout,
                _layout.rectangle(),
                background_color(),
                focus_color(),
                layout().theme->border_width,
                hi::border_side::inside,
                layout().theme->rounding_radius);
        }
    }

    // Override this function when your widget needs to be controllable by keyboard interaction.
    [[nodiscard]] bool accepts_keyboard_focus(hi::keyboard_focus_group group) const noexcept override
    {
        // This widget will react to "normal" tab/shift-tab keys and mouse clicks to focus the widget.
        return *mode >= hi::widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal);
    }

    // Override this function when your widget needs to be controllable by mouse interaction.
    [[nodiscard]] hi::hitbox hitbox_test(hi::point3 position) const noexcept override
    {
        // Check if the (mouse) position is within the visual-area of the widget.
        // The hit_rectangle is the _layout.rectangle() intersected with the _layout.clipping_rectangle.
        if (*mode >= hi::widget_mode::partial and layout().contains(position)) {
            // The `this` argument allows the gui_window to forward mouse events to handle_event(mouse) of this widget.
            // The `position` argument is used to handle widgets that are visually overlapping, widgets with higher elevation
            // get priority. When this widget is enabled it should show a button-cursor, otherwise just the normal arrow.
            return {this, position, *mode >= hi::widget_mode::partial ? hi::hitbox_type::button : hi::hitbox_type::_default};

        } else {
            return {};
        }
    }

    // Override the handle_event(command) to handle high level commands.
    bool handle_event(hi::gui_event const& event) noexcept override
    {
        switch (event.type()) {
        case hi::gui_event_type::gui_activate:
            if (*mode >= hi::widget_mode::partial) {
                // Handle activate, by default the "spacebar" causes this command.
                value = not *value;
                return true;
            }
            break;

        case hi::gui_event_type::keyboard_grapheme:
            hi_log_error("User typed the letter U+{:x}.", static_cast<uint32_t>(get<0>(event.grapheme())));
            return true;

        case hi::gui_event_type::mouse_up:
            if (*mode >= hi::widget_mode::partial and event.is_left_button_up(_layout.rectangle())) {
                return handle_event(hi::gui_event_type::gui_activate);
            }
            break;

        default:;
        }

        // The default handle_event() will handle hovering and auto-scrolling.
        return widget::handle_event(event);
    }

private:
    decltype(value)::callback_token _value_cbt;
};

int hi_main(int argc, char *argv[])
{
    auto gui = hi::gui_system::make_unique();
    auto window = gui->make_window(hi::tr("Custom Widget Command"));
    window->content().make_widget<command_widget>("A1");
    window->content().make_widget<command_widget>("A2");

    auto close_cbt = window->closing.subscribe(
        [&] {
            window = {};
        },
        hi::callback_flags::main);
    return hi::loop::main().resume();
}
