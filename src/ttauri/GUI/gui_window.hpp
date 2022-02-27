// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window_size.hpp"
#include "gui_window_delegate.hpp"
#include "mouse_cursor.hpp"
#include "hitbox.hpp"
#include "mouse_event.hpp"
#include "keyboard_event.hpp"
#include "keyboard_focus_direction.hpp"
#include "keyboard_focus_group.hpp"
#include "theme.hpp"
#include "../GFX/subpixel_orientation.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../widgets/window_widget.hpp"
#include "../widgets/grid_widget.hpp"
#include "../widgets/toolbar_widget.hpp"
#include "../chrono.hpp"
#include "../label.hpp"
#include "../animator.hpp"
#include <unordered_set>
#include <memory>
#include <mutex>

namespace tt::inline v1 {
class gfx_device;
class gfx_system;
class gfx_surface;
class gui_system;
class keyboard_bindings;

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class gui_window {
public:
    using delegate_type = gui_window_delegate;

    gui_system &gui;

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

    label title;

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi = 72.0;

    /** Theme to use to draw the widgets on this window.
     * The sizes and colors of the theme have already been adjusted to the window's state and dpi.
     */
    tt::theme theme = {};

    /** The size of the widget.
     */
    extent2 widget_size;

    /** The widget covering the complete window.
     */
    std::unique_ptr<window_widget> widget;

    gui_window(gui_system &gui, label const &title, std::weak_ptr<delegate_type> delegate = {}) noexcept;

    virtual ~gui_window();

    gui_window(gui_window const &) = delete;
    gui_window &operator=(gui_window const &) = delete;
    gui_window(gui_window &&) = delete;
    gui_window &operator=(gui_window &&) = delete;

    /** 2 phase constructor.
     * Must be called directly after the constructor on the same thread,
     * before another thread can send messages to the window.
     *
     * `init()` should not take locks on window::mutex.
     */
    virtual void init();

    /** 2 phase constructor.
     * Must be called directly before the destructor on the same thread,
     *
     * `deinit()` should not take locks on window::mutex.
     */
    virtual void deinit();

    /** Check if the current thread is the same as the gui_system loop.
     */
    [[nodiscard]] bool is_gui_thread() const noexcept;

    void set_device(gfx_device *device) noexcept;

    /** Get the keyboard binding.
     */
    tt::keyboard_bindings const &keyboard_bindings() const noexcept;

    /** Request a rectangle on the window to be redrawn
     */
    void request_redraw(aarectangle redraw_rectangle) noexcept
    {
        _redraw_rectangle |= redraw_rectangle;
    }

    /** Request a rectangle on the window to be redrawn
     */
    void request_redraw() noexcept
    {
        tt_axiom(is_gui_thread());
        request_redraw(aarectangle{rectangle.size()});
    }

    void request_relayout() noexcept
    {
        _relayout.store(true, std::memory_order::relaxed);
    }

    void request_reconstrain() noexcept
    {
        _reconstrain.store(true, std::memory_order::relaxed);
    }

    void request_resize() noexcept
    {
        _resize.store(true, std::memory_order::relaxed);
    }

    /** Update window.
     * This will update animations and redraw all widgets managed by this window.
     */
    virtual void render(utc_nanoseconds displayTimePoint);

    /** Check if the window was closed by the operating system.
     */
    [[nodiscard]] bool is_closed() const noexcept;

    /** Get a reference to the window's content widget.
     * @see grid_widget
     * @return A reference to a grid_widget.
     */
    [[nodiscard]] grid_widget &content() noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(widget);
        return widget->content();
    }

    /** Get a reference to window's toolbar widget.
     * @see toolbar_widget
     * @return A reference to a toolbar_widget.
     */
    [[nodiscard]] toolbar_widget &toolbar() noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(widget);
        return widget->toolbar();
    }

    /** Set the mouse cursor icon.
     */
    virtual void set_cursor(mouse_cursor cursor) = 0;

    void set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept;

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

    virtual tt::subpixel_orientation subpixel_orientation() const noexcept = 0;

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

    /** Retrieve a text string from the operating system's clip-board.
     */
    [[nodiscard]] virtual std::string get_text_from_clipboard() const noexcept = 0;

    /** Place a text string on the operating system's clip-board.
     */
    virtual void set_text_on_clipboard(std::string str) noexcept = 0;

    void update_mouse_target(tt::widget const *new_target_widget, point2 position = {}) noexcept;

    /** Change the keyboard focus to the given widget.
     * If the group of the widget is incorrect then no widget will be in focus.
     *
     * @param widget The new widget to focus, or empty to remove all keyboard focus.
     * @param group The group the widget must belong to.
     */
    void update_keyboard_target(tt::widget const *widget, keyboard_focus_group group = keyboard_focus_group::normal) noexcept;

    /** Change the keyboard focus to the previous or next widget from the given widget.
     * This function will find the closest widget from the given widget which belongs to the given
     * group; if none is found, or if the original selected widget is found, then no widget will be in focus.
     *
     * @param widget The widget to use as the start point for a new widget to select.
     * @param group The group the widget must belong to.
     * @param direction The direction to search in, or current to select the current widget.
     */
    void
    update_keyboard_target(tt::widget const *widget, keyboard_focus_group group, keyboard_focus_direction direction) noexcept;

    /** Change the keyboard focus to the given, previous or next widget.
     * This function will find the closest widget from the current widget which belongs to the given
     * group; if none is found, or if the original selected widget is found, then no widget will be in focus.
     *
     * @param group The group the widget must belong to.
     * @param direction The direction to search in, or current to select the current widget.
     */
    void update_keyboard_target(keyboard_focus_group group, keyboard_focus_direction direction) noexcept;

    [[nodiscard]] translate2 window_to_screen() const noexcept
    {
        return translate2{rectangle.left(), rectangle.bottom()};
    }

    [[nodiscard]] translate2 screen_to_window() const noexcept
    {
        return ~window_to_screen();
    }

protected:
    static constexpr std::chrono::nanoseconds _animation_duration = std::chrono::milliseconds(150);

    std::weak_ptr<delegate_type> _delegate;

    std::atomic<aarectangle> _redraw_rectangle = aarectangle{};
    std::atomic<bool> _relayout = true;
    std::atomic<bool> _reconstrain = true;
    std::atomic<bool> _resize = true;

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

    /** Handle command event.
     * This function is called when no widget has handled the command.
     */
    [[nodiscard]] virtual bool handle_event(tt::command command) noexcept;

    [[nodiscard]] virtual bool handle_event(std::vector<tt::command> const &commands) noexcept
    {
        for (ttlet command : commands) {
            if (handle_event(command)) {
                return true;
            }
        }
        return false;
    }

    /** Handle mouse event.
     * This function is called when no widget has handled the mouse event.
     */
    [[nodiscard]] virtual bool handle_event(mouse_event const &event) noexcept
    {
        return false;
    }

    /** Handle keyboard event.
     * This function is called when no widget has handled the keyboard event.
     */
    [[nodiscard]] virtual bool handle_event(keyboard_event const &event) noexcept
    {
        return false;
    }

    /** Send Mouse event.
     * Called by the operating system to show the position of the mouse.
     * This is called very often so it must be made efficient.
     * Most often this function is used to determine the mouse cursor.
     */
    bool send_event(mouse_event const &event) noexcept;

    /*! Handle keyboard event.
     * Called by the operating system to show the character that was entered
     * or special key that was used.
     */
    bool send_event(keyboard_event const &event) noexcept;

    bool send_event(KeyboardState _state, keyboard_modifiers modifiers, keyboard_virtual_key key) noexcept;

    bool send_event(grapheme grapheme, bool full = true) noexcept;

private:
    std::shared_ptr<std::function<void()>> _setting_change_callback;
    std::shared_ptr<std::function<void()>> _selected_theme_callback;
    /** Target of the mouse
     * Since any mouse event will change the target this is used
     * to check if the target has changed, to send exit events to the previous mouse target.
     */
    tt::widget const *_mouse_target_widget = nullptr;

    /** Target of the keyboard
     * widget where keyboard events are sent to.
     */
    tt::widget const *_keyboard_target_widget = nullptr;

    /** Called when a widget is being destructed.
     * This removes internal references to widgets.
     * Particularly the mouse and keyboard targets.
     */
    void widget_is_destructing(tt::widget const *sender) noexcept
    {
        if (_mouse_target_widget == sender) {
            _mouse_target_widget = nullptr;
        }
        if (_keyboard_target_widget == sender) {
            _keyboard_target_widget = nullptr;
        }
    }

    /** Send event to a target widget.
     *
     * The commands are send in order, until the command is handled, then processing stops immediately.
     * All commands are tried in a batch to the following handlers:
     *  - The target widget
     *  - The parents of the widget up to and including the root widget.
     *  - The window itself.
     */
    template<typename Event>
    bool send_event_to_widget(tt::widget const *target_widget, Event const &event) noexcept;

    friend class widget;
};

} // namespace tt::inline v1
