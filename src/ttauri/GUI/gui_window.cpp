// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_window.hpp"
#include "gui_system.hpp"
#include "keyboard_bindings.hpp"
#include "../GFX/gfx_device.hpp"
#include "../GFX/gfx_surface.hpp"
#include "../trace.hpp"
#include "../widgets/window_widget.hpp"
#include "../widgets/grid_widget.hpp"

namespace tt {

template<typename Event>
bool gui_window::send_event_to_widget(tt::widget const *target_widget, Event const &event) noexcept
{
    while (target_widget) {
        // Send a command in priority order to the widget.
        if constexpr (std::is_same_v<Event, mouse_event>) {
            if (const_cast<tt::widget *>(target_widget)->handle_event(target_widget->window_to_local() * event)) {
                return true;
            }

        } else {
            if (const_cast<tt::widget *>(target_widget)->handle_event(event)) {
                return true;
            }
        }

        // Forward the keyboard event to the parent of the target.
        target_widget = target_widget->parent;
    }

    // If non of the widget has handled the command, let the window handle the command.
    if (handle_event(event)) {
        return true;
    }
    return false;
}

gui_window::gui_window(gui_system &gui, label const &title, std::weak_ptr<gui_window_delegate> delegate) noexcept :
    gui(gui), title(title), _delegate(std::move(delegate))
{
}

gui_window::~gui_window()
{
    // Destroy the top-level widget, before Window-members that the widgets require from the window during their destruction.
    widget = {};

    try {
        surface.reset();
        tt_log_info("Window '{}' has been properly destructed.", title);

    } catch (std::exception const &e) {
        tt_log_fatal("Could not properly destruct gui_window. '{}'", e.what());
    }
}

void gui_window::init()
{
    // This function is called just after construction in single threaded mode,
    // and therefor should not have a lock.
    tt_axiom(is_gui_thread());

    widget = std::make_unique<window_widget>(*this, title, _delegate);
    if (auto delegate = _delegate.lock()) {
        delegate->init(*this);
    }

    // Execute a constraint check to determine initial window size.
    static_cast<void>(widget->constrain({}, true));
    ttlet new_size = widget->preferred_size();

    // Reset the keyboard target to not focus anything.
    update_keyboard_target({});

    _setting_change_callback = language::subscribe([this] {
        this->request_reconstrain();
    });

    // Delegate has been called, layout of widgets has been calculated for the
    // minimum and maximum size of the window.
    create_window(new_size);
}

void gui_window::deinit()
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

[[nodiscard]] bool gui_window::is_gui_thread() const noexcept
{
    return gui.is_gui_thread();
}

void gui_window::set_device(gfx_device *device) noexcept
{
    tt_axiom(surface);
    surface->set_device(device);
}

[[nodiscard]] bool gui_window::is_closed() const noexcept
{
    return surface->is_closed();
}

[[nodiscard]] float gui_window::window_scale() const noexcept
{
    tt_axiom(is_gui_thread());

    return std::ceil(dpi / 100.0f);
}

void gui_window::render(utc_nanoseconds displayTimePoint)
{
    tt_axiom(is_gui_thread());
    tt_axiom(surface);
    tt_axiom(widget);

    // All widgets need constrains recalculated on these window-wide events.
    // Like theme or language changes.
    ttlet need_reconstrain = _reconstrain.exchange(false);

    // Update the size constraints of the window_widget and it children.
    ttlet constraints_have_changed = widget->constrain(displayTimePoint, need_reconstrain);

    // Check if the window size matches the preferred size of the window_widget.
    // If not ask the operating system to change the size of the window, which is
    // done asynchronously.
    //
    // We need to continue drawing into the incorrectly sized window, otherwise
    // Vulkan will not detect the change of drawing surface's size.
    //
    // Make sure the widget does have its window rectangle match the constraints, otherwise
    // the logic for layout and drawing becomes complicated.

    if (request_resize.exchange(false)) {
        // If a widget asked for a resize, change the size of the window to the preferred size of the widgets.
        ttlet current_size = screen_rectangle.size();
        ttlet new_size = widget->preferred_size();
        if (new_size != current_size) {
            tt_log_info("A new preferred window size {} was requested by one of the widget.", new_size);
            set_window_size(new_size);
        }

    } else {
        // Check if the window size matches the minimum and maximum size of the widgets, otherwise resize. 
        ttlet current_size = screen_rectangle.size();
        ttlet new_size = clamp(current_size, widget->minimum_size(), widget->maximum_size());
        if (new_size != current_size and size_state != gui_window_size::minimized) {
            tt_log_info("The current window size {} must grow or shrink to {} to fit the widgets.", current_size, new_size);
            set_window_size(new_size);
        }
    }

    if (screen_rectangle.size() < widget->minimum_size() or screen_rectangle.size() > widget->maximum_size()) {
        // Even after the resize above it is possible to have an incorrect window size.
        // For example when minimizing the window.
        // Stop processing rendering for this window here.
        return;
    }

    // Update the graphics' surface to the current size of the window.
    surface->update(screen_rectangle.size());

    widget->set_layout_parameters_from_parent(aarectangle{screen_rectangle.size()});

    // When a window message was received, such as a resize, redraw, language-change; the request_layout is set to true.
    ttlet need_layout = _relayout.exchange(false, std::memory_order::relaxed) or constraints_have_changed;

    // Make sure the widget's layout is updated before draw, but after window resize.
    if (need_layout or widget_size != screen_rectangle.size()) {
        widget_size = screen_rectangle.size();
        widget->layout(geo::identity{}, widget_size, displayTimePoint, need_layout);
    }

    if (auto optional_draw_context = surface->render_start(_redraw_rectangle)) {
        auto draw_context = *optional_draw_context;
        auto tr = trace<"window_render">();

        _redraw_rectangle = aarectangle{};

        auto widget_context =
            draw_context.make_child_context(widget->parent_to_local(), widget->local_to_window(), widget->clipping_rectangle());

        widget->draw(widget_context, displayTimePoint);

        surface->render_finish(draw_context, widget->background_color());
    }
}

void gui_window::set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
{
    tt_axiom(is_gui_thread());
    tt_axiom(widget);
    return widget->set_resize_border_priority(left, right, bottom, top);
}

void gui_window::update_mouse_target(tt::widget const *new_target_widget, point2 position) noexcept
{
    tt_axiom(is_gui_thread());

    if (new_target_widget != _mouse_target_widget) {
        if (_mouse_target_widget) {
            if (!send_event_to_widget(_mouse_target_widget, mouse_event::exited())) {
                send_event_to_widget(_mouse_target_widget, std::vector{command::gui_mouse_exit});
            }
        }
        _mouse_target_widget = new_target_widget;
        if (new_target_widget) {
            if (!send_event_to_widget(new_target_widget, mouse_event::entered(position))) {
                send_event_to_widget(new_target_widget, std::vector{command::gui_mouse_enter});
            }
        }
    }
}

tt::keyboard_bindings const &gui_window::keyboard_bindings() const noexcept
{
    tt_axiom(gui.keyboard_bindings);
    return *gui.keyboard_bindings;
}

void gui_window::update_keyboard_target(tt::widget const *new_target_widget, keyboard_focus_group group) noexcept
{
    tt_axiom(is_gui_thread());

    // Before we are going to make new_target_widget empty, due to the rules below;
    // capture which parents there are.
    auto new_target_parent_chain = new_target_widget ? new_target_widget->parent_chain() : std::vector<tt::widget const *>{};

    // If the new target widget does not accept focus, for example when clicking
    // on a disabled widget, or empty part of a window.
    // In that case no widget will get focus.
    if (not new_target_widget or not new_target_widget->accepts_keyboard_focus(group)) {
        new_target_widget = {};
    }

    // Check if the keyboard focus changed.
    if (new_target_widget == _keyboard_target_widget) {
        return;
    }

    // When there is a new target, tell the current widget that the keyboard focus was exited.
    if (new_target_widget and _keyboard_target_widget) {
        send_event_to_widget(_keyboard_target_widget, std::vector{command::gui_keyboard_exit});
        _keyboard_target_widget = nullptr;
    }

    // Tell "escape" to all the widget that are not parents of the new widget
    [[maybe_unused]] ttlet handled = widget->handle_command_recursive(command::gui_escape, new_target_parent_chain);

    // Tell the new widget that keyboard focus was entered.
    if (new_target_widget) {
        _keyboard_target_widget = new_target_widget;
        send_event_to_widget(new_target_widget, std::vector{command::gui_keyboard_enter});
    }
}

void gui_window::update_keyboard_target(
    tt::widget const *start_widget,
    keyboard_focus_group group,
    keyboard_focus_direction direction) noexcept
{
    tt_axiom(is_gui_thread());

    auto tmp = widget->find_next_widget(start_widget, group, direction);
    if (tmp == start_widget) {
        // Could not a next widget, loop around.
        tmp = widget->find_next_widget({}, group, direction);
    }
    update_keyboard_target(tmp, group);
}

void gui_window::update_keyboard_target(keyboard_focus_group group, keyboard_focus_direction direction) noexcept
{
    update_keyboard_target(_keyboard_target_widget, group, direction);
}

bool gui_window::handle_event(tt::command command) noexcept
{
    switch (command) {
    case command::gui_widget_next:
        update_keyboard_target(_keyboard_target_widget, keyboard_focus_group::normal, keyboard_focus_direction::forward);
        return true;
    case command::gui_widget_prev:
        update_keyboard_target(_keyboard_target_widget, keyboard_focus_group::normal, keyboard_focus_direction::backward);
        return true;
    default:;
    }
    return false;
}

bool gui_window::send_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());

    switch (event.type) {
    case mouse_event::Type::Exited: // Mouse left window.
        update_mouse_target({});
        break;

    case mouse_event::Type::ButtonDown:
    case mouse_event::Type::Move: {
        ttlet hitbox = widget->hitbox_test(event.position);
        update_mouse_target(hitbox.widget, event.position);

        if (event.type == mouse_event::Type::ButtonDown) {
            update_keyboard_target(hitbox.widget, keyboard_focus_group::any);
        }
    } break;
    default:;
    }

    if (send_event_to_widget(_mouse_target_widget, event)) {
        return true;
    }

    return false;
}

bool gui_window::send_event(keyboard_event const &event) noexcept
{
    tt_axiom(is_gui_thread());

    if (send_event_to_widget(_keyboard_target_widget, event)) {
        return true;
    }

    // If the keyboard event is not handled directly, convert the key event to a command.
    if (event.type == keyboard_event::Type::Key) {
        ttlet commands = keyboard_bindings().translate(event.key);

        ttlet handled = send_event_to_widget(_keyboard_target_widget, commands);

        for (ttlet command : commands) {
            // Intercept the keyboard generated escape.
            // A keyboard generated escape should always remove keyboard focus.
            // The update_keyboard_target() function will send gui_keyboard_exit and a
            // potential duplicate gui_escape messages to all widgets that need it.
            if (command == command::gui_escape) {
                update_keyboard_target({}, keyboard_focus_group::any);
            }
        }

        return handled;
    }

    return false;
}

bool gui_window::send_event(KeyboardState _state, keyboard_modifiers modifiers, keyboard_virtual_key key) noexcept
{
    return send_event(keyboard_event(_state, modifiers, key));
}

bool gui_window::send_event(grapheme grapheme, bool full) noexcept
{
    return send_event(keyboard_event(grapheme, full));
}

bool gui_window::send_event(char32_t c, bool full) noexcept
{
    return send_event(grapheme(c), full);
}

} // namespace tt
