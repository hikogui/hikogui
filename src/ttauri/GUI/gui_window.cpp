// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_window.hpp"
#include "gui_device.hpp"
#include "keyboard_bindings.hpp"
#include "../widgets/window_widget.hpp"

namespace tt {

template<typename Event>
bool gui_window::send_event_to_widget(std::shared_ptr<tt::widget> target_widget, Event const &event) noexcept
{
    while (target_widget) {
        // Send a command in priority order to the widget.
        if constexpr (std::is_same_v<Event,mouse_event>) {
            if (target_widget->handle_event(target_widget->window_to_local() * event)) {
                return true;
            }

        } else {
            if (target_widget->handle_event(event)) {
                return true;
            }
        }

        // Forward the keyboard event to the parent of the target.
        target_widget = target_widget->shared_parent();
    }

    // If non of the widget has handled the command, let the window handle the command.
    if (handle_event(event)) {
        return true;
    }
    return false;
}

gui_window::gui_window(gui_system &system, std::weak_ptr<gui_window_delegate> const &delegate, label const &title) :
    system(system), state(gui_window_state::initializing), delegate(delegate), title(title)
{
}

gui_window::~gui_window()
{
    // Destroy the top-level widget, before Window-members that the widgets require from the window during their destruction.
    widget = {};

    try {
        if (state != gui_window_state::no_window) {
            tt_log_fatal("Window '{}' was not properly teardown before destruction.", title);
        }
        tt_log_info("Window '{}' has been propertly destructed.", title);

    } catch (std::exception const &e) {
        tt_log_fatal("Could not properly destruct gui_window. '{}'", e.what());
    }
}

void gui_window::init()
{
    // This function is called just after construction in single threaded mode,
    // and therefor should not have a lock on the window.
    tt_assert(is_main_thread(), "createWindow should be called from the main thread.");
    tt_axiom(gui_system_mutex.recurse_lock_count() == 0);

    widget = std::make_shared<window_widget>(*this, delegate, title);
    widget->init();

    // The delegate will populate the window with widgets.
    // This needs to be done first to figure out the initial size of the window.
    if (auto delegate_ = delegate.lock()) {
        delegate_->init(*this);
    }

    // Execute a constraint check to determine initial window size.
    extent = [this] {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        static_cast<void>(widget->update_constraints({}, true));
        return widget->preferred_size().minimum();
    }();

    // Once the window is open, we should be a full constraint, layout and draw of the window.
    requestLayout = true;

    // Reset the keyboard target to not focus anything.
    update_keyboard_target({});

    // Finished initializing the window.
    state = gui_window_state::no_device;

    // Delegate has been called, layout of widgets has been calculated for the
    // minimum and maximum size of the window.
    create_window(title.text(), extent);
}

void gui_window::set_device(gui_device *new_device)
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (_device == new_device) {
        return;
    }

    if (new_device) {
        // The assigned device must be from the same GUI-system.
        tt_assert(&system == &new_device->system);
    }

    if (_device) {
        state = gui_window_state::device_lost;
        teardown();
    }

    _device = new_device;
}

bool gui_window::is_closed()
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    return state == gui_window_state::no_window;
}

[[nodiscard]] float gui_window::window_scale() const noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    return std::ceil(dpi / 100.0f);
}

void gui_window::window_changed_size(extent2 new_extent)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    extent = new_extent;

    tt_axiom(widget);
    widget->set_layout_parameters_from_parent(aarect{extent});
    requestLayout = true;
}

void gui_window::set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    tt_axiom(widget);
    return widget->set_resize_border_priority(left, right, bottom, top);
}

void gui_window::update_mouse_target(std::shared_ptr<tt::widget> new_target_widget, point2 position) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto current_target_widget = _mouse_target_widget.lock();
    if (new_target_widget != current_target_widget) {
        if (current_target_widget) {
            if (!send_event_to_widget(current_target_widget, mouse_event::exited())) {
                send_event_to_widget(current_target_widget, std::vector{command::gui_mouse_exit});
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

void gui_window::update_keyboard_target(std::shared_ptr<tt::widget> new_target_widget, keyboard_focus_group group) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    // If the new target widget does not accept focus, for example when clicking
    // on a disabled widget, or empty part of a window.
    // In that case no widget will get focus.
    if (!new_target_widget || !new_target_widget->accepts_keyboard_focus(group)) {
        new_target_widget = {};
    }

    // Check if the keyboard focus changed.
    ttlet current_target_widget = _keyboard_target_widget.lock();
    if (new_target_widget == current_target_widget) {
        return;
    }

    // Tell the current widget that the keyboard focus was exited.
    if (current_target_widget) {
        send_event_to_widget(current_target_widget, std::vector{command::gui_keyboard_exit});
    }

    // Send a gui_cancel command to any widget that is not in the new_target_widget-parent-chain.
    auto new_target_parent_chain = tt::widget::parent_chain(new_target_widget);
    widget->handle_command_recursive(command::gui_escape, new_target_parent_chain);

    // Tell the new widget that keyboard focus was entered.
    _keyboard_target_widget = new_target_widget;
    if (new_target_widget) {
        send_event_to_widget(new_target_widget, std::vector{command::gui_keyboard_enter});
    }
}

void gui_window::update_keyboard_target(
    std::shared_ptr<tt::widget> const &start_widget,
    keyboard_focus_group group,
    keyboard_focus_direction direction) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    if (direction == keyboard_focus_direction::current) {
        update_keyboard_target(std::move(start_widget), group);
    } else {
        auto tmp = widget->find_next_widget(start_widget, group, direction);
        if (tmp == start_widget) {
            // The currentTargetWidget was already the last (or only) widget;
            // don't focus anything.
            tmp = {};
        }
        update_keyboard_target(std::move(tmp), group);
    }
}

bool gui_window::handle_event(tt::command command) noexcept
{
    switch (command) {
    case command::gui_widget_next:
        update_keyboard_target(_keyboard_target_widget.lock(), keyboard_focus_group::normal, keyboard_focus_direction::forward);
        return true;
    case command::gui_widget_prev:
        update_keyboard_target(_keyboard_target_widget.lock(), keyboard_focus_group::normal, keyboard_focus_direction::backward);
        return true;
    default:;
    }
    return false;
}



/*[[nodiscard]] bool gui_window::send_event(std::shared_ptr<tt::widget> target_widget, mouse_event const &event) noexcept
{
    tt::send_event(target_widget, event);
}

[[nodiscard]] bool gui_window::send_event(std::shared_ptr<tt::widget> target_widget, keyboard_event const &event) noexcept
{
    tt::send_event(target_widget, event);
}

[[nodiscard]] bool
gui_window::send_event(std::shared_ptr<tt::widget> target_widget, std::vector<tt::command> const &event) noexcept
{
    tt::send_event(target_widget, event);
}*/

bool gui_window::send_event(mouse_event const &event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    switch (event.type) {
    case mouse_event::Type::Exited: // Mouse left window.
        update_mouse_target({});
        break;

    case mouse_event::Type::ButtonDown:
    case mouse_event::Type::Move: {
        ttlet hitbox = widget->hitbox_test(event.position);
        update_mouse_target(std::const_pointer_cast<tt::widget>(hitbox.widget.lock()), event.position);

        if (event.type == mouse_event::Type::ButtonDown) {
            update_keyboard_target(std::const_pointer_cast<tt::widget>(hitbox.widget.lock()), keyboard_focus_group::any);
        }
    } break;
    default:;
    }

    auto target = _mouse_target_widget.lock();

    if (send_event_to_widget(target, event)) {
        return true;
    }

    return false;
}

bool gui_window::send_event(keyboard_event const &event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    auto target = _keyboard_target_widget.lock();

    if (send_event_to_widget(target, event)) {
        return true;
    }

    // If the keyboard event is not handled directly, convert the key event to a command.
    if (event.type == keyboard_event::Type::Key) {
        ttlet commands = keyboardBindings.translate(event.key);

        ttlet handled = send_event_to_widget(target, commands);

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
