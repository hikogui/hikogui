// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_window.hpp"
#include "gui_system.hpp"
#include "keyboard_bindings.hpp"
#include "theme_book.hpp"
#include "../os_settings.hpp"
#include "../GFX/gfx_device.hpp"
#include "../GFX/gfx_surface.hpp"
#include "../widgets/window_widget.hpp"
#include "../widgets/grid_widget.hpp"
#include "../trace.hpp"
#include "../log.hpp"

namespace tt::inline v1 {

template<typename Event>
bool gui_window::send_event_to_widget(tt::widget const *target_widget, Event const &event) noexcept
{
    while (target_widget) {
        // Send a command in priority order to the widget.
        if constexpr (std::is_same_v<Event, mouse_event>) {
            if (const_cast<tt::widget *>(target_widget)->handle_event(target_widget->layout().from_window * event)) {
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
    theme = gui.theme_book->find(*gui.selected_theme.cget(), os_settings::theme_mode()).transform(dpi);
    ttlet new_size = widget->set_constraints().preferred;

    // Reset the keyboard target to not focus anything.
    update_keyboard_target({});

    // For changes in setting on the OS we should reconstrain/layout/redraw the window
    // For example when the language or theme changes.
    _setting_change_callback = os_settings::subscribe([this] {
        this->request_reconstrain();
    });

    // Subscribe on theme changes.
    _selected_theme_callback = gui.selected_theme.subscribe([this] {
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

void gui_window::render(utc_nanoseconds display_time_point)
{
    ttlet t1 = trace<"window::render">();

    tt_axiom(is_gui_thread());
    tt_axiom(surface);
    tt_axiom(widget);

    // When a widget requests it or a window-wide event like language change
    // has happened all the widgets will be set_constraints().
    auto need_reconstrain = _reconstrain.exchange(false, std::memory_order_relaxed);

#if 0
    // For performance checks force reconstrain.
    need_reconstrain = true;
#endif

    if (need_reconstrain) {
        ttlet t2 = trace<"window::constrain">();

        theme = gui.theme_book->find(*gui.selected_theme.cget(), os_settings::theme_mode()).transform(dpi);

        widget->set_constraints();
    }

    // Check if the window size matches the preferred size of the window_widget.
    // If not ask the operating system to change the size of the window, which is
    // done asynchronously.
    //
    // We need to continue drawing into the incorrectly sized window, otherwise
    // Vulkan will not detect the change of drawing surface's size.
    //
    // Make sure the widget does have its window rectangle match the constraints, otherwise
    // the logic for layout and drawing becomes complicated.
    if (_resize.exchange(false)) {
        // If a widget asked for a resize, change the size of the window to the preferred size of the widgets.
        ttlet current_size = rectangle.size();
        ttlet new_size = widget->constraints().preferred;
        if (new_size != current_size) {
            tt_log_info("A new preferred window size {} was requested by one of the widget.", new_size);
            set_window_size(new_size);
        }

    } else {
        // Check if the window size matches the minimum and maximum size of the widgets, otherwise resize.
        ttlet current_size = rectangle.size();
        ttlet new_size = clamp(current_size, widget->constraints().minimum, widget->constraints().maximum);
        if (new_size != current_size and size_state() != gui_window_size::minimized) {
            tt_log_info("The current window size {} must grow or shrink to {} to fit the widgets.", current_size, new_size);
            set_window_size(new_size);
        }
    }

    if (rectangle.size() < widget->constraints().minimum or rectangle.size() > widget->constraints().maximum) {
        // Even after the resize above it is possible to have an incorrect window size.
        // For example when minimizing the window.
        // Stop processing rendering for this window here.
        return;
    }

    // Update the graphics' surface to the current size of the window.
    surface->update(rectangle.size());

    // Make sure the widget's layout is updated before draw, but after window resize.
    auto need_relayout = _relayout.exchange(false, std::memory_order_relaxed);

#if 0
    // For performance checks force relayout.
    need_relayout = true;
#endif

    if (need_reconstrain or need_relayout or widget_size != rectangle.size()) {
        ttlet t2 = trace<"window::layout">();
        widget_size = rectangle.size();

        // Guarantee that the layout size is always at least the minimum size.
        // We do this because it simplifies calculations if no minimum checks are necessary inside widget.
        ttlet widget_layout_size = max(widget->constraints().minimum, widget_size);
        widget->set_layout(
            widget_layout{widget_layout_size, this->subpixel_orientation(), gui.writing_direction, display_time_point});

        // After layout do a complete redraw.
        _redraw_rectangle = aarectangle{widget_size};
    }

#if 0
    // For performance checks force redraw.
    _redraw_rectangle = aarectangle{widget_size};
#endif

    // Draw widgets if the _redraw_rectangle was set.
    if (auto draw_context = surface->render_start(_redraw_rectangle)) {
        _redraw_rectangle = aarectangle{};
        draw_context.display_time_point = display_time_point;
        draw_context.subpixel_orientation = subpixel_orientation();
        draw_context.background_color = widget->background_color();

        if (_animated_active.update(active ? 1.0f : 0.0f, display_time_point)) {
            request_redraw();
        }
        draw_context.saturation = _animated_active.current_value();

        {
            ttlet t2 = trace<"window::draw">();
            widget->draw(draw_context);
        }
        {
            ttlet t2 = trace<"window::submit">();
            surface->render_finish(draw_context);
        }
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
    widget->handle_command_recursive(command::gui_cancel, new_target_parent_chain);

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
    case command::gui_toolbar_open:
        update_keyboard_target(widget.get(), keyboard_focus_group::toolbar, keyboard_focus_direction::forward);
        return true;
    case command::text_edit_copy:
        // Widgets, other than the current keyboard target may have text selected and can handle the command::text_edit_copy.
        widget->handle_command_recursive(command::text_edit_copy);
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
            update_keyboard_target(hitbox.widget, keyboard_focus_group::all);
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
            // potential duplicate gui_cancel messages to all widgets that need it.
            if (command == command::gui_cancel) {
                update_keyboard_target({}, keyboard_focus_group::all);
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

} // namespace tt::inline v1
