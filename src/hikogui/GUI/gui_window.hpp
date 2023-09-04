// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window_size.hpp"
#include "mouse_cursor.hpp"
#include "hitbox.hpp"
#include "gui_event.hpp"
#include "keyboard_focus_direction.hpp"
#include "keyboard_focus_group.hpp"
#include "theme_book.hpp"
#include "keyboard_bindings.hpp"
#include "widget_intf.hpp"
#include "../unicode/unicode.hpp"
#include "../GFX/GFX.hpp"
#include "../geometry/module.hpp"
#include "../time/module.hpp"
#include "../l10n/l10n.hpp"
#include "../algorithm/module.hpp"
#include "../macros.hpp"
#include <unordered_set>
#include <memory>
#include <mutex>

namespace hi::inline v1 {
class gui_system;

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class gui_window {
public:
    std::unique_ptr<gfx_surface> surface;

    /** The current rectangle of the window relative to the screen.
     * The screen rectangle is set by the operating system event loop.
     *
     * This rectangle is used by the operating system event loop hit-testing
     * to determine the position of screen coordinates to window coordinates.
     *
     * The size of this rectangle is used to laying out widgets and setting
     * the size of the gfx_surface during rendering.
     */
    aarectangle rectangle;

    /** The current cursor.
     * Used for optimizing when the operating system cursor is updated.
     * Set to mouse_cursor::None at the start (for the wait icon) and when the
     * operating system is going to display another icon to make sure
     * when it comes back in the application the cursor will be updated
     * correctly.
     */
    mouse_cursor current_mouse_cursor = mouse_cursor::None;

    /*! The window is currently being resized by the user.
     * We can disable expensive redraws during rendering until this
     * is false again.
     */
    bool resizing = false;

    /*! The window is currently active.
     * Widgets may want to reduce redraws, or change colors.
     */
    bool active = false;

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi = 72.0;

    /** Theme to use to draw the widgets on this window.
     * The sizes and colors of the theme have already been adjusted to the window's state and dpi.
     */
    hi::theme theme = {};

    /** The size of the widget.
     */
    extent2 widget_size;

    /** Notifier used when the window is closing.
     * It is expected that after notifying these callbacks the instance of this class is destroyed.
     */
    notifier<void()> closing;

    gui_window(std::unique_ptr<widget_intf> widget) noexcept : _widget(std::move(widget))
    {
        if (_first_window) {
            if (not os_settings::start_subsystem()) {
                hi_log_fatal("Could not start the os_settings subsystem.");
            }

            register_font_file(URL{"resource:fonts/elusiveicons-webfont.ttf"});
            register_font_file(URL{"resource:fonts/hikogui_icons.ttf"});
            register_font_directories(get_paths(path_location::font_dirs));

            register_theme_directories(get_paths(path_location::theme_dirs));

            try {
                load_system_keyboard_bindings(URL{"resource:win32.keybinds.json"});
            } catch (std::exception const& e) {
                hi_log_fatal("Could not load keyboard bindings. \"{}\"", e.what());
            }

            _first_window = true;
        }
    }

    virtual ~gui_window()
    {
        // Destroy the top-level widget, before Window-members that the widgets require from the window during their destruction.
        _widget = {};

        try {
            surface.reset();
            hi_log_info("Window '{}' has been properly destructed.", _title);

        } catch (std::exception const& e) {
            hi_log_fatal("Could not properly destruct gui_window. '{}'", e.what());
        }
    }

    gui_window(gui_window const&) = delete;
    gui_window& operator=(gui_window const&) = delete;
    gui_window(gui_window&&) = delete;
    gui_window& operator=(gui_window&&) = delete;

    /** Update window.
     * This will update animations and redraw all widgets managed by this window.
     */
    virtual void render(utc_nanoseconds display_time_point)
    {
        if (surface->device() == nullptr) {
            // If there is no device configured for the surface don't try to render.
            return;
        }

        hilet t1 = trace<"window::render">();

        hi_axiom(loop::main().on_thread());
        hi_assert_not_null(surface);
        hi_assert_not_null(_widget);

        // When a widget requests it or a window-wide event like language change
        // has happened all the widgets will be set_constraints().
        auto need_reconstrain = _reconstrain.exchange(false, std::memory_order_relaxed);

#if 0
    // For performance checks force reconstrain.
    need_reconstrain = true;
#endif

        if (need_reconstrain) {
            hilet t2 = trace<"window::constrain">();

            theme = get_selected_theme().transform(dpi);

            _widget_constraints = _widget->update_constraints();
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
        if (_resize.exchange(false, std::memory_order::relaxed)) {
            // If a widget asked for a resize, change the size of the window to the preferred size of the widgets.
            hilet current_size = rectangle.size();
            hilet new_size = _widget_constraints.preferred;
            if (new_size != current_size) {
                hi_log_info("A new preferred window size {} was requested by one of the widget.", new_size);
                set_window_size(new_size);
            }

        } else {
            // Check if the window size matches the minimum and maximum size of the widgets, otherwise resize.
            hilet current_size = rectangle.size();
            hilet new_size = clamp(current_size, _widget_constraints.minimum, _widget_constraints.maximum);
            if (new_size != current_size and size_state() != gui_window_size::minimized) {
                hi_log_info("The current window size {} must grow or shrink to {} to fit the widgets.", current_size, new_size);
                set_window_size(new_size);
            }
        }

        if (rectangle.size() < _widget_constraints.minimum or rectangle.size() > _widget_constraints.maximum) {
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
            hilet t2 = trace<"window::layout">();
            widget_size = rectangle.size();

            // Guarantee that the layout size is always at least the minimum size.
            // We do this because it simplifies calculations if no minimum checks are necessary inside widget.
            hilet widget_layout_size = max(_widget_constraints.minimum, widget_size);
            _widget->set_layout(widget_layout{widget_layout_size, _size_state, subpixel_orientation(), display_time_point});

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
            draw_context.active = active;

            if (_animated_active.update(active ? 1.0f : 0.0f, display_time_point)) {
                this->process_event({gui_event_type::window_redraw, aarectangle{rectangle.size()}});
            }
            draw_context.saturation = _animated_active.current_value();

            {
                hilet t2 = trace<"window::draw">();
                _widget->draw(draw_context);
            }
            {
                hilet t2 = trace<"window::submit">();
                surface->render_finish(draw_context);
            }
        }
    }

    template<typename Widget>
    [[nodiscard]] Widget& widget() const noexcept
    {
        return up_cast<Widget>(*_widget);
    }

    virtual void set_title(label title) noexcept
    {
        _title = std::move(title);
    }

    /** Set the mouse cursor icon.
     */
    virtual void set_cursor(mouse_cursor cursor) = 0;

    /** Ask the operating system to close this window.
     */
    virtual void close_window() = 0;

    /** Set the size-state of the window.
     *
     * This function is used to change the size of the window to one
     * of the predefined states: normal, minimized, maximized or full-screen.
     */
    virtual void set_size_state(gui_window_size state) noexcept = 0;

    /** The rectangle of the workspace of the screen where the window is currently located.
     */
    virtual aarectangle workspace_rectangle() const noexcept = 0;

    /** The rectangle of the screen where the window is currently located.
     */
    virtual aarectangle fullscreen_rectangle() const noexcept = 0;

    virtual hi::subpixel_orientation subpixel_orientation() const noexcept = 0;

    /** Get the size-state of the window.
     */
    gui_window_size size_state() const noexcept
    {
        return _size_state;
    }

    /** Open the system menu of the window.
     *
     * On windows 10 this is activated by pressing Alt followed by Spacebar.
     */
    virtual void open_system_menu() = 0;

    /** Ask the operating system to set the size of this window.
     */
    virtual void set_window_size(extent2 extent) = 0;

    void update_mouse_target(widget_id new_target_id, point2 position = {}) noexcept
    {
        hi_axiom(loop::main().on_thread());

        if (_mouse_target_id) {
            if (new_target_id == _mouse_target_id) {
                // Focus does not change.
                return;
            }

            // The mouse target needs to be updated, send exit to previous target.
            send_events_to_widget(_mouse_target_id, std::vector{gui_event{gui_event_type::mouse_exit}});
        }

        if (new_target_id) {
            _mouse_target_id = new_target_id;
            send_events_to_widget(new_target_id, std::vector{gui_event::make_mouse_enter(position)});
        } else {
            _mouse_target_id = std::nullopt;
        }
    }

    /** Change the keyboard focus to the given widget.
     * If the group of the widget is incorrect then no widget will be in focus.
     *
     * @param widget The new widget to focus, or empty to remove all keyboard focus.
     * @param group The group the widget must belong to.
     */
    void update_keyboard_target(widget_id new_target_id, keyboard_focus_group group = keyboard_focus_group::normal) noexcept
    {
        hi_axiom(loop::main().on_thread());

        auto new_target_widget = get_if(_widget.get(), new_target_id, false);

        // Before we are going to make new_target_widget empty, due to the rules below;
        // capture which parents there are.
        auto new_target_parent_chain = new_target_widget ? new_target_widget->parent_chain() : std::vector<widget_id>{};

        // If the new target widget does not accept focus, for example when clicking
        // on a disabled widget, or empty part of a window.
        // In that case no widget will get focus.
        if (new_target_widget == nullptr or not new_target_widget->accepts_keyboard_focus(group)) {
            new_target_widget = nullptr;
        }

        if (auto const *const keyboard_target_widget = get_if(_widget.get(), _keyboard_target_id, false)) {
            // keyboard target still exists and visible.
            if (new_target_widget == keyboard_target_widget) {
                // Focus does not change.
                return;
            }

            send_events_to_widget(_keyboard_target_id, std::vector{gui_event{gui_event_type::keyboard_exit}});
        }

        // Tell "escape" to all the widget that are not parents of the new widget
        _widget->handle_event_recursive(gui_event_type::gui_cancel, new_target_parent_chain);

        // Tell the new widget that keyboard focus was entered.
        if (new_target_widget != nullptr) {
            _keyboard_target_id = new_target_widget->id;
            send_events_to_widget(_keyboard_target_id, std::vector{gui_event{gui_event_type::keyboard_enter}});
        } else {
            _keyboard_target_id = std::nullopt;
        }
    }

    /** Change the keyboard focus to the previous or next widget from the given widget.
     * This function will find the closest widget from the given widget which belongs to the given
     * group; if none is found, or if the original selected widget is found, then no widget will be in focus.
     *
     * @param start_widget The widget to use as the start point for a new widget to select.
     * @param group The group the widget must belong to.
     * @param direction The direction to search in, or current to select the current widget.
     */
    void update_keyboard_target(widget_id start_widget, keyboard_focus_group group, keyboard_focus_direction direction) noexcept
    {
        hi_axiom(loop::main().on_thread());

        auto tmp = _widget->find_next_widget(start_widget, group, direction);
        if (tmp == start_widget) {
            // Could not a next widget, loop around.
            tmp = _widget->find_next_widget({}, group, direction);
        }
        update_keyboard_target(tmp, group);
    }

    /** Change the keyboard focus to the given, previous or next widget.
     * This function will find the closest widget from the current widget which belongs to the given
     * group; if none is found, or if the original selected widget is found, then no widget will be in focus.
     *
     * @param group The group the widget must belong to.
     * @param direction The direction to search in, or current to select the current widget.
     */
    void update_keyboard_target(keyboard_focus_group group, keyboard_focus_direction direction) noexcept
    {
        return update_keyboard_target(_keyboard_target_id, group, direction);
    }

    /** Get text from the clipboard.
     *
     * @note This is part of the window as some operating systems need to know from which window the text was posted.
     * @return The text from the clipboard.
     * @retval empty When the clipboard is locked by another application, on error, if the data on the clipboard can not
     *               be converted to text or if the clipboard is empty.
     */
    [[nodiscard]] virtual std::optional<gstring> get_text_from_clipboard() const noexcept = 0;

    /** Put text on the clipboard.
     *
     * @note This is part of the window as some operating systems need to know from which window the text was posted.
     * @param text The text to place on the clipboard.
     */
    virtual void put_text_on_clipboard(gstring_view text) const noexcept = 0;

    [[nodiscard]] translate2 window_to_screen() const noexcept
    {
        return translate2{rectangle.left(), rectangle.bottom()};
    }

    [[nodiscard]] translate2 screen_to_window() const noexcept
    {
        return ~window_to_screen();
    }

    /** Process the event.
     *
     * This is called by the event handler to start processing events.
     * The events are translated and then uses `send_event_to_widget()` to send the
     * events to the widgets in some priority ordering.
     *
     * It may also be called from within the `event_handle()` of widgets.
     */
    bool process_event(gui_event const& event) noexcept
    {
        using enum gui_event_type;

        hi_axiom(loop::main().on_thread());

        auto events = std::vector<gui_event>{event};

        switch (event.type()) {
        case window_redraw:
            _redraw_rectangle.fetch_or(event.rectangle());
            return true;

        case window_relayout:
            _relayout.store(true, std::memory_order_relaxed);
            return true;

        case window_reconstrain:
            _reconstrain.store(true, std::memory_order_relaxed);
            return true;

        case window_resize:
            _resize.store(true, std::memory_order_relaxed);
            return true;

        case window_minimize:
            set_size_state(gui_window_size::minimized);
            return true;

        case window_maximize:
            set_size_state(gui_window_size::maximized);
            return true;

        case window_normalize:
            set_size_state(gui_window_size::normal);
            return true;

        case window_close:
            close_window();
            return true;

        case window_open_sysmenu:
            open_system_menu();
            return true;

        case window_set_keyboard_target:
            {
                hilet& target = event.keyboard_target();
                if (target.widget_id == nullptr) {
                    update_keyboard_target(target.group, target.direction);
                } else if (target.direction == keyboard_focus_direction::here) {
                    update_keyboard_target(target.widget_id, target.group);
                } else {
                    update_keyboard_target(target.widget_id, target.group, target.direction);
                }
            }
            return true;

        case window_set_clipboard:
            put_text_on_clipboard(event.clipboard_data());
            return true;

        case mouse_exit_window: // Mouse left window.
            update_mouse_target({});
            break;

        case mouse_down:
        case mouse_move:
            {
                hilet hitbox = _widget->hitbox_test(event.mouse().position);
                update_mouse_target(hitbox.widget_id, event.mouse().position);

                if (event == mouse_down) {
                    update_keyboard_target(hitbox.widget_id, keyboard_focus_group::all);
                }
            }
            break;

        case keyboard_down:
            for (auto& e : translate_keyboard_event(event)) {
                events.push_back(e);
            }
            break;

        default:;
        }

        for (auto& event_ : events) {
            if (event_.type() == gui_event_type::text_edit_paste) {
                // The text-edit-paste operation was generated by keyboard bindings,
                // it needs the actual text to be pasted added.
                if (auto optional_text = get_text_from_clipboard()) {
                    event_.clipboard_data() = *optional_text;
                }
            }
        }

        hilet handled = [&] {
            hilet target_id = event.variant() == gui_event_variant::mouse ? _mouse_target_id : _keyboard_target_id;
            return send_events_to_widget(target_id, events);
        }();

        // Intercept the keyboard generated escape.
        // A keyboard generated escape should always remove keyboard focus.
        // The update_keyboard_target() function will send gui_keyboard_exit and a
        // potential duplicate gui_cancel messages to all widgets that need it.
        for (hilet event_ : events) {
            if (event_ == gui_cancel) {
                update_keyboard_target({}, keyboard_focus_group::all);
            }
        }

        return handled;
    }

protected:
    constexpr static std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    /** The label of the window that is passed to the operating system.
     */
    label _title;

    /** The widget covering the complete window.
     */
    std::unique_ptr<widget_intf> _widget;

    box_constraints _widget_constraints = {};

    std::atomic<aarectangle> _redraw_rectangle = aarectangle{};
    std::atomic<bool> _relayout = false;
    std::atomic<bool> _reconstrain = false;
    std::atomic<bool> _resize = false;

    /** Current size state of the window.
     */
    gui_window_size _size_state = gui_window_size::normal;

    /** When the window is minimized, maximized or made full-screen the original size is stored here.
     */
    aarectangle _restore_rectangle;

    /** The time of the last forced redraw.
     * A forced redraw may happen when needing to draw outside
     * of the event-loop. For example when win32 moving or resizing
     * the event-loop is stuck, so forced redraws are happening on
     * the WM_MOVING and WM_SIZING messages that are generated outside
     * the event loop, but on the same thread as the event loop.
     */
    utc_nanoseconds last_forced_redraw = {};

    /** The animated version of the `active` flag.
     */
    animator<float> _animated_active = _animation_duration;

    /** Let the operating system create the actual window.
     * @pre title and extent must be set.
     */
    virtual void create_window(extent2 new_size) = 0;

private:
    inline static bool _first_window = true;

    /** Target of the mouse
     * Since any mouse event will change the target this is used
     * to check if the target has changed, to send exit events to the previous mouse target.
     */
    widget_id _mouse_target_id;

    /** Target of the keyboard
     * widget where keyboard events are sent to.
     */
    widget_id _keyboard_target_id;

    /** Send event to a target widget.
     *
     * The commands are send in order, until the command is handled, then processing stops immediately.
     * All commands are tried in a batch to the following handlers:
     *  - The target widget
     *  - The parents of the widget up to and including the root widget.
     *  - The window itself.
     */
    bool send_events_to_widget(widget_id target_id, std::vector<gui_event> const& events) noexcept
    {
        if (not target_id) {
            // If there was no target, send the event to the window's widget.
            target_id = _widget->id;
        }

        auto target_widget = get_if(_widget.get(), target_id, false);
        while (target_widget) {
            // Each widget will try to handle the first event it can.
            for (hilet& event : events) {
                if (target_widget->handle_event(target_widget->layout().from_window * event)) {
                    return true;
                }
            }

            // Forward the events to the parent of the target.
            target_widget = target_widget->parent;
        }

        return false;
    }

    friend class widget;
};

/** Create a new window with the specified managing widget.
 *
 * @param widget The widget that manages the window.
 * @return A owning pointer to the new window.
 *         releasing the pointer will close the window.
 */
[[nodiscard]] std::unique_ptr<gui_window> make_unique_window_with_widget(std::unique_ptr<widget_intf> widget);

/** Create a new window.
 *
 * @tparam Widget The type of widget to create to manage the window.
 * @param args The arguments that are forwarded to the constructor of the managing
 *             widget that is created.
 * @return A owning pointer to the new window.
 *         releasing the pointer will close the window.
 */
template<std::derived_from<widget_intf> Widget, typename... Args>
[[nodiscard]] inline std::pair<std::unique_ptr<gui_window>, Widget&> make_unique_window(Args&&...args)
{
    auto widget = std::make_unique<Widget>(std::forward<Args>(args)...);
    auto& widget_ref = *widget;

    return {make_unique_window_with_widget(std::move(widget)), widget_ref};
}

} // namespace hi::inline v1
