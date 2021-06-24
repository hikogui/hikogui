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
#include "../text/gstring.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../hires_utc_clock.hpp"
#include "../label.hpp"
#include <unordered_set>
#include <memory>
#include <mutex>

namespace tt {
class gfx_device;
class gfx_system;
class window_widget;
class gfx_surface;

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class gui_window {
public:
    std::unique_ptr<gfx_surface> surface;

    /** The current cursor.
     * Used for optimizing when the operating system cursor is updated.
     * Set to mouse_cursor::None at the start (for the wait icon) and when the
     * operating system is going to display another icon to make sure
     * when it comes back in the application the cursor will be updated
     * correctly.
     */
    mouse_cursor currentmouse_cursor = mouse_cursor::None;

    /** When set to true the widgets will be laid out.
     */
    std::atomic<bool> requestLayout = true;

    /** When set to true the window will resize to the size of the contained widget.
     */
    std::atomic<bool> requestResize = true;

    /*! The window is currently being resized by the user.
     * We can disable expensive redraws during rendering until this
     * is false again.
     */
    bool resizing = false;

    /*! The window is currently active.
     * Widgets may want to reduce redraws, or change colors.
     */
    bool active = false;

    /*! Current size state of the window.
     */
    gui_window_size size_state = gui_window_size::normal;

    /** The size from the surface, clamped to combined widget's size.
     */
    extent2 size;

    label title;

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi = 72.0;

    //! The widget covering the complete window.
    std::unique_ptr<window_widget> widget;

    gui_window(std::shared_ptr<gui_window_delegate> delegate, label const &title);
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

    void set_device(gfx_device *device) noexcept;

    /** Request a rectangle on the window to be redrawn
     */
    void request_redraw(aarectangle rectangle) noexcept
    {
        _request_redraw_rectangle |= rectangle;
    }

    /** Request a rectangle on the window to be redrawn
     */
    void request_redraw() noexcept
    {
        tt_axiom(is_gui_thread());
        request_redraw(aarectangle{size});
    }

    /** By how much the font needs to be scaled compared to current windowScale.
     * Widgets should pass this value to the text-shaper.
     */
    [[nodiscard]] float fontScale() const noexcept
    {
        return dpi / (window_scale() * 72.0f);
    }

    /** Update window.
     * This will update animations and redraw all widgets managed by this window.
     */
    virtual void render(hires_utc_clock::time_point displayTimePoint);

    /** Check if the window was closed by the operating system.
     */
    [[nodiscard]] bool is_closed() const noexcept;

    /** Add a widget to main widget of the window.
     * The implementation is in widgets.hpp
     */
    template<typename T, typename... Args>
    T &make_widget(size_t column_nr, size_t row_nr, Args &&...args);

    /** Add a widget to main widget of the window.
     * The implementation is in widgets.hpp
     */
    template<typename T, typename... Args>
    T &make_widget(std::string_view address, Args &&...args);

    /** Add a widget to main widget of the window.
     * The implementation is in widgets.hpp
     */
    template<typename T, horizontal_alignment Alignment = horizontal_alignment::left, typename... Args>
    T &make_toolbar_widget(Args &&...args);

    /** Set the mouse cursor icon.
     */
    virtual void set_cursor(mouse_cursor cursor) = 0;

    void set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept;

    /** Ask the operating system to close this window.
     */
    virtual void close_window() = 0;

    /** Ask the operating system to minimize this window.
     */
    virtual void minimize_window() = 0;

    /** Ask the operating system to maximize this window.
     */
    virtual void maximize_window() = 0;

    /** Ask the operating system to normalize this window.
     */
    virtual void normalize_window() = 0;

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
    void update_keyboard_target(tt::widget const *widget, keyboard_focus_group group, keyboard_focus_direction direction) noexcept;

    /** Change the keyboard focus to the given, previous or next widget.
     * This function will find the closest widget from the current widget which belongs to the given
     * group; if none is found, or if the original selected widget is found, then no widget will be in focus.
     *
     * @param group The group the widget must belong to.
     * @param direction The direction to search in, or current to select the current widget.
     */
    void update_keyboard_target(keyboard_focus_group group, keyboard_focus_direction direction) noexcept;

    /** Get the size of the virtual-screen.
     * Each window may be on a different virtual screen with different
     * sizes, so retrieve it on a per window basis.
     */
    [[nodiscard]] virtual extent2 virtual_screen_size() const noexcept = 0;

    [[nodiscard]] translate2 window_to_screen() const noexcept
    {
        return translate2{_screen_rectangle.left(), _screen_rectangle.bottom()};
    }

    [[nodiscard]] translate2 screen_to_window() const noexcept
    {
        return ~window_to_screen();
    }

protected:
    std::shared_ptr<gui_window_delegate> _delegate;

    /*! The current rectangle of the window relative to the screen.
     * The screen rectangle is set by the operating system event loop and
     * the extent of the rectangle may lag behind the actual window extent as seen
     * by the GPU library.
     *
     * This rectangle is used by the operating system event loop hit-testing
     * to determine the position of screen coordinates to window coordinates.
     *
     * It may also be used for the extent of the window when the GPU
     * library is unable to determine the extent of the surface.
     */
    aarectangle _screen_rectangle;

    std::atomic<bool> _request_setting_change = true;

    std::atomic<aarectangle> _request_redraw_rectangle = aarectangle{};

    /** The time of the last forced redraw.
     * A forced redraw may happen when needing to draw outside
     * of the event-loop. For example when win32 moving or resizing
     * the event-loop is stuck, so forced redraws are happening on
     * the WM_MOVING and WM_SIZING messages that are generated outside
     * the event loop, but on the same thread as the event loop.
     */
    hires_utc_clock::time_point last_forced_redraw = {};

    /** Let the operating system create the actual window.
     * @pre title and extent must be set.
     */
    virtual void create_window() = 0;

    /** By how much graphic elements should be scaled to match a point.
     * The widget should not care much about this value, since the
     * transformation matrix will match the window scaling.
     */
    [[nodiscard]] float window_scale() const noexcept;

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

    bool send_event(char32_t c, bool full = true) noexcept;

private:
    std::shared_ptr<std::function<void()>> _setting_change_callback;

    /** Target of the mouse
     * Since any mouse event will change the target this is used
     * to check if the target has changed, to send exit events to the previous mouse target.
     */
    tt::widget const *_mouse_target_widget = nullptr;

    /** Target of the keyboard
     * widget where keyboard events are sent to.
     */
    tt::widget const *_keyboard_target_widget = nullptr;

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
};

} // namespace tt
