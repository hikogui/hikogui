// Copyright 2019 Pokitec
// All rights reserved.

#include "gui_window.hpp"
#include "gui_device.hpp"
#include "../widgets/WindowWidget.hpp"

namespace tt {

using namespace std;

gui_window::gui_window(gui_system &system, std::weak_ptr<gui_window_delegate> const &delegate, label const &title) :
    system(system), state(State::Initializing), delegate(delegate), title(title)
{
}

gui_window::~gui_window()
{
    // Destroy the top-level widget, before Window-members that the widgets require from the window during their destruction.
    widget = {};

    try {
        if (state != State::NoWindow) {
            LOG_FATAL("Window '{}' was not properly teardown before destruction.", title);
        }
        LOG_INFO("Window '{}' has been propertly destructed.", title);

    } catch (std::exception const &e) {
        LOG_FATAL("Could not properly destruct gui_window {}", to_string(e));
    }
}

void gui_window::init()
{
    // This function is called just after construction in single threaded mode,
    // and therefor should not have a lock on the window.
    tt_assert(is_main_thread(), "createWindow should be called from the main thread.");
    tt_axiom(gui_system_mutex.recurse_lock_count() == 0);

    widget = std::make_shared<WindowWidget>(*this, delegate, title);
    widget->init();

    // The delegate will populate the window with widgets.
    // This needs to be done first to figure out the initial size of the window.
    if (auto delegate_ = delegate.lock()) {
        delegate_->init(*this);
    }

    // Execute a constraint check to determine initial window size.
    currentWindowExtent = [this] {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        static_cast<void>(widget->update_constraints({}, true));
        return widget->preferred_size().minimum();
    }();

    // Once the window is open, we should be a full constraint, layout and draw of the window.
    requestLayout = true;

    // Reset the keyboard target to not focus anything.
    update_keyboard_target({});

    // Finished initializing the window.
    state = State::NoDevice;

    // Delegate has been called, layout of widgets has been calculated for the
    // minimum and maximum size of the window.
    createWindow(title.text(), currentWindowExtent);
}

void gui_window::setDevice(gui_device *new_device)
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
        state = State::DeviceLost;
        teardown();
    }

    _device = new_device;
}

bool gui_window::isClosed()
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    return state == State::NoWindow;
}

[[nodiscard]] float gui_window::windowScale() const noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    return std::ceil(dpi / 100.0f);
}

void gui_window::windowChangedSize(f32x4 extent)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    currentWindowExtent = extent;
    tt_axiom(widget);

    widget->set_layout_parameters(aarect{currentWindowExtent}, aarect{currentWindowExtent});
    requestLayout = true;
}

void gui_window::set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    tt_axiom(widget);
    return widget->set_resize_border_priority(left, right, bottom, top);
}

void gui_window::update_mouse_target(std::shared_ptr<tt::widget> new_target_widget, f32x4 position) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto current_target_widget = mouseTargetWidget.lock();
    if (new_target_widget != current_target_widget) {
        if (current_target_widget) {
            current_target_widget->handle_mouse_event(MouseEvent::exited());
        }
        mouseTargetWidget = new_target_widget;
        if (new_target_widget) {
            new_target_widget->handle_mouse_event(MouseEvent::entered(position));
        }
    }
}

void gui_window::update_keyboard_target(std::shared_ptr<tt::widget> new_target_widget, keyboard_focus_group group) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    // Send a gui_cancel command to any widget that is not in the new_target_widget-parent-chain.
    auto new_target_parent_chain = tt::widget::parent_chain(new_target_widget);
    widget->handle_command_recursive(command::gui_escape, new_target_parent_chain);

    auto current_target_widget = keyboardTargetWidget.lock();
    if ((!new_target_widget || new_target_widget->accepts_keyboard_focus(group)) && new_target_widget != current_target_widget) {
        if (current_target_widget) {
            current_target_widget->handle_keyboard_event(KeyboardEvent::exited());
        }
        keyboardTargetWidget = new_target_widget;
        if (new_target_widget) {
            new_target_widget->handle_keyboard_event(KeyboardEvent::entered());
        }
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

bool gui_window::handle_mouse_event(MouseEvent event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    switch (event.type) {
    case MouseEvent::Type::Exited: // Mouse left window.
        update_mouse_target({});
        break;

    case MouseEvent::Type::ButtonDown:
    case MouseEvent::Type::Move: {
        ttlet hitbox = widget->hitbox_test(event.position);
        update_mouse_target(std::const_pointer_cast<tt::widget>(hitbox.widget.lock()), event.position);

        if (event.type == MouseEvent::Type::ButtonDown) {
            update_keyboard_target(std::const_pointer_cast<tt::widget>(hitbox.widget.lock()), keyboard_focus_group::any);
        }
    } break;
    default:;
    }

    auto target = mouseTargetWidget.lock();
    while (target) {
        if (target->handle_mouse_event(event)) {
            return true;
        }

        // Forward the mouse event to the parent of the target.
        target = target->parent.lock();
    }

    return false;
}

bool gui_window::handle_keyboard_event(KeyboardEvent const &event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    // Let the widget or its parent handle the keyboard event directly.
    {
        auto target = keyboardTargetWidget.lock();
        while (target) {
            if (target->handle_keyboard_event(event)) {
                return true;
            }
            // Forward the keyboard event to the parent of the target.
            target = target->parent.lock();
        }
    }

    // If the keyboard event is not handled directly, convert the key event to a command.
    if (event.type == KeyboardEvent::Type::Key) {
        ttlet commands = event.getCommands();

        // Send the commands to the widget and its parents, until the command is handled.
        {
            auto target = keyboardTargetWidget.lock();
            while (target) {
                for (auto command : commands) {
                    // Send a command in priority order to the widget.
                    if (target->handle_command(command)) {
                        return true;
                    }
                }
                // Forward the keyboard event to the parent of the target.
                target = target->parent.lock();
            }
        }

        // If no widgets handle the commands, handle the keyboard focus change commands.
        for (ttlet command : commands) {
            switch (command) {
            case command::gui_widget_next:
                update_keyboard_target(
                    keyboardTargetWidget.lock(), keyboard_focus_group::normal, keyboard_focus_direction::forward);
                return true;
            case command::gui_widget_prev:
                update_keyboard_target(
                    keyboardTargetWidget.lock(), keyboard_focus_group::normal, keyboard_focus_direction::backward);
                return true;
            default:;
            }
        }
    }

    return false;
}

bool gui_window::handle_keyboard_event(KeyboardState _state, KeyboardModifiers modifiers, KeyboardVirtualKey key) noexcept
{
    return handle_keyboard_event(KeyboardEvent(_state, modifiers, key));
}

bool gui_window::handle_keyboard_event(Grapheme grapheme, bool full) noexcept
{
    return handle_keyboard_event(KeyboardEvent(grapheme, full));
}

bool gui_window::handle_keyboard_event(char32_t c, bool full) noexcept
{
    return handle_keyboard_event(Grapheme(c), full);
}

} // namespace tt
