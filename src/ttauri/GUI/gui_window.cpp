// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_window.hpp"
#include "gui_device.hpp"
#include "gui_surface.hpp"
#include "keyboard_bindings.hpp"
#include "../trace.hpp"
#include "../widgets/window_widget.hpp"
#include "../widgets/grid_layout_widget.hpp"

namespace tt {

template<typename Event>
bool gui_window::send_event_to_widget(std::shared_ptr<tt::widget> target_widget, Event const &event) noexcept
{
    while (target_widget) {
        // Send a command in priority order to the widget.
        if constexpr (std::is_same_v<Event, mouse_event>) {
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

gui_window::gui_window(gui_system &system, std::shared_ptr<gui_window_delegate> delegate, label const &title) :
    system(system), _delegate(std::move(delegate)), title(title)
{
}

gui_window::~gui_window()
{
    // Destroy the top-level widget, before Window-members that the widgets require from the window during their destruction.
    widget = {};

    try {
        surface.reset();
        tt_log_info("Window '{}' has been propertly destructed.", title);

    } catch (std::exception const &e) {
        tt_log_fatal("Could not properly destruct gui_window. '{}'", e.what());
    }
}

void gui_window::init()
{
    // This function is called just after construction in single threaded mode,
    // and therefor should not have a lock.
    tt_assert(is_main_thread(), "createWindow should be called from the main thread.");
    tt_axiom(gui_system_mutex.recurse_lock_count() == 0);

    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        widget = std::make_shared<window_widget>(*this, _delegate, title);
        widget->init();
        _delegate->init(*this);

        // Execute a constraint check to determine initial window size.
        static_cast<void>(widget->update_constraints({}, true));
        size = widget->preferred_size();

        // Reset the keyboard target to not focus anything.
        update_keyboard_target({});

        _setting_change_callback = language::subscribe([this]() {
            ttlet lock = std::scoped_lock(gui_system_mutex);
            this->_request_setting_change = true;
        });
    }

    // Delegate has been called, layout of widgets has been calculated for the
    // minimum and maximum size of the window.
    create_window();
}

void gui_window::deinit()
{
    _delegate->deinit(*this);
}

void gui_window::set_device(gui_device *device) noexcept
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
    ttlet lock = std::scoped_lock(gui_system_mutex);

    return std::ceil(dpi / 100.0f);
}

void gui_window::render(hires_utc_clock::time_point displayTimePoint)
{
    tt_axiom(gui_system_mutex.recurse_lock_count());
    tt_axiom(surface);
    tt_axiom(widget);

    // All widgets need constrains recalculated on these window-wide events.
    // Like theme or language changes.
    ttlet need_reconstrain = std::exchange(_request_setting_change, false);

    // Update the size constraints of the window_widget and it children.
    ttlet constraints_have_changed = widget->update_constraints(displayTimePoint, need_reconstrain);

    // Check if the window size matches the preferred size of the window_widget.
    // If not ask the operating system to change the size of the window, which is
    // done asynchronously.
    //
    // We need to continue drawing into the incorrectly sized window, otherwise
    // Vulkan will not detect the change of drawing surface's size.
    //
    // Make sure the widget does have its window rectangle match the constraints, otherwise
    // the logic for layout and drawing becomes complicated.
    {
        ttlet new_size = surface->update(widget->minimum_size(), widget->maximum_size());
        if (new_size != size) {
            requestLayout = true;
            size = new_size;
        }
    }

    if (requestResize.exchange(false)) {
        tt_log_info("A new preferred window size {} was requested by one of the widget.", widget->preferred_size());
        set_window_size(size = widget->preferred_size());
    } else {
        ttlet new_size = clamp(size, widget->minimum_size(), widget->maximum_size());
        if (new_size != size) {
            tt_log_info("The current window size {} must grow or shrink to {} to fit the widgets.", size, new_size);
            set_window_size(size = new_size);
        }
    }
    widget->set_layout_parameters_from_parent(aarectangle{size});

    // When a window message was received, such as a resize, redraw, language-change; the requestLayout is set to true.
    ttlet need_layout = requestLayout.exchange(false, std::memory_order::relaxed) || constraints_have_changed;

    // Make sure the widget's layout is updated before draw, but after window resize.
    widget->update_layout(displayTimePoint, need_layout);

    if (auto optional_draw_context = surface->render_start(_request_redraw_rectangle)) {
        auto draw_context = *optional_draw_context;
        auto tr = trace<"window_render", "frame_buffer_index">();

        _request_redraw_rectangle = aarectangle{};

        auto widget_context =
            draw_context.make_child_context(widget->parent_to_local(), widget->local_to_window(), widget->clipping_rectangle());

        widget->draw(widget_context, displayTimePoint);

        surface->render_finish(draw_context, widget->backgroundColor());
    }
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

    // Before we are going to make new_target_widget empty, due to the rules below;
    // capture which parents there are.
    auto new_target_parent_chain = tt::widget::parent_chain(new_target_widget);

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

    // When there is a new target, tell the current widget that the keyboard focus was exited.
    if (new_target_widget && current_target_widget) {
        send_event_to_widget(current_target_widget, std::vector{command::gui_keyboard_exit});
        _keyboard_target_widget = {};
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
    std::shared_ptr<tt::widget> const &start_widget,
    keyboard_focus_group group,
    keyboard_focus_direction direction) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    auto tmp = widget->find_next_widget(start_widget, group, direction);
    if (tmp == start_widget) {
        // Could not a next widget, loop around.
        tmp = widget->find_next_widget({}, group, direction);
    }
    update_keyboard_target(std::move(tmp), group);
}

void gui_window::update_keyboard_target(keyboard_focus_group group, keyboard_focus_direction direction) noexcept
{
    auto current_keyboard_widget = _keyboard_target_widget.lock();
    update_keyboard_target(current_keyboard_widget, group, direction);
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
