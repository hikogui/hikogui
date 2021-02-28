// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window_state.hpp"
#include "gui_window_size.hpp"
#include "gui_window_delegate.hpp"
#include "gui_system_globals.hpp"
#include "mouse_cursor.hpp"
#include "hit_box.hpp"
#include "mouse_event.hpp"
#include "keyboard_event.hpp"
#include "subpixel_orientation.hpp"
#include "keyboard_focus_direction.hpp"
#include "keyboard_focus_group.hpp"
#include "../text/gstring.hpp"
#include "../logger.hpp"
#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include "../cpu_utc_clock.hpp"
#include "../cell_address.hpp"
#include "../label.hpp"
#include <unordered_set>
#include <memory>
#include <mutex>

namespace tt {
class gui_device;
class gui_system;
class window_widget;

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class gui_window {
public:
    gui_system &system;

    gui_window_state state = gui_window_state::no_device;

    /** The current cursor.
     * Used for optimizing when the operating system cursor is updated.
     * Set to mouse_cursor::None at the start (for the wait icon) and when the
     * operating system is going to display another icon to make sure
     * when it comes back in the application the cursor will be updated
     * correctly.
     */
    mouse_cursor currentmouse_cursor = mouse_cursor::None;

    /** When set to true the widgets will be layed out.
     */
    std::atomic<bool> requestLayout = true;

    /** When set to true the window will resize to the size of the contained widget.
     */
    std::atomic<bool> requestResize = true;

    /*! The window is currently being resized by the user.
     * We can disable expensive redraws during rendering until this
     * is false again.
     */
    std::atomic<bool> resizing = false;

    /*! The window is currently active.
     * Widgets may want to reduce redraws, or change colors.
     */
    std::atomic<bool> active = false;

    /*! Current size state of the window.
     */
    gui_window_size size_state = gui_window_size::normal;

    //! The current window extent as set by the GPU library.
    extent2 extent;

    std::weak_ptr<gui_window_delegate> delegate;

    label title;

    /*! Orientation of the RGB subpixels.
     */
    subpixel_orientation subpixel_orientation = subpixel_orientation::BlueRight;

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi = 72.0;

    //! The widget covering the complete window.
    std::shared_ptr<window_widget> widget;

    gui_window(gui_system &system, std::weak_ptr<gui_window_delegate> const &delegate, label const &title);
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

    /** Request a rectangle on the window to be redrawn
     */
    void request_redraw(aarect rectangle = aarect::infinity()) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        _request_redraw_rectangle |= rectangle;
    }

    /** By how much the font needs to be scaled compared to current windowScale.
     * Widgets should pass this value to the text-shaper.
     */
    [[nodiscard]] float fontScale() const noexcept
    {
        return dpi / (window_scale() * 72.0f);
    }

    /*! Set GPU device to manage this window.
     * Change of the device may be done at runtime.
     */
    void set_device(gui_device *device);

    /*! Remove the GPU device from the window, making it an orphan.
     */
    void unset_device()
    {
        set_device({});
    }

    gui_device *device() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return _device;
    }

    /** Update window.
     * This will update animations and redraw all widgets managed by this window.
     */
    virtual void render(hires_utc_clock::time_point displayTimePoint) = 0;

    /** Check if the window was closed by the operating system.
     */
    bool is_closed();

    /** Add a widget to main widget of the window.
     * The implementation is in widgets.hpp
     */
    template<typename T, cell_address CellAddress, typename... Args>
    std::shared_ptr<T> make_widget(Args &&...args);

    /** Add a widget to main widget of the window.
     * The implementation is in widgets.hpp
     */
    template<typename T, horizontal_alignment Alignment = horizontal_alignment::left, typename... Args>
    std::shared_ptr<T> make_toolbar_widget(Args &&...args);

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

    void update_mouse_target(std::shared_ptr<tt::widget> new_target_widget, point2 position = {}) noexcept;

    /** Change the keyboard focus to the given widget.
     * If the group of the widget is incorrect then no widget will be in focus.
     *
     * @param widget The new widget to focus, or empty to remove all keyboard focus.
     * @param group The group the widget must belong to.
     */
    void update_keyboard_target(
        std::shared_ptr<tt::widget> widget,
        keyboard_focus_group group = keyboard_focus_group::normal) noexcept;

    /** Change the keyboard focus to the given, previous or next widget.
     * This function will find the closest widget from the given widget wich belong to the given
     * group; if none is found, or if the original selected widget is found, then no widget will be in focus.
     *
     * @param widget The widget to use as the start point for a new widget to select.
     * @param group The group the widget must belong to.
     * @param direction The direction to search in, or current to select the current widget.
     */
    void update_keyboard_target(
        std::shared_ptr<tt::widget> const &widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) noexcept;

    /** Get the size of the virtual-screen.
     * Each window may be on a different virtual screen with different
     * sizes, so retrieve it on a per window basis.
     */
    [[nodiscard]] virtual extent2 virtual_screen_size() const noexcept = 0;

protected:
    /** The device the window is assigned to.
     * The device may change during the lifetime of a window,
     * as long as the device belongs to the same GUIInstance.
     */
    gui_device *_device = nullptr;

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
    aarect _screen_rectangle;

    bool _request_setting_change = true;

    aarect _request_redraw_rectangle = aarect{};

    /** Let the operating system create the actual window.
     * @param title The title of the window.
     * @param extent The size of the window.
     */
    virtual void create_window(const std::u8string &title, extent2 extent) = 0;

    /** By how much graphic elements should be scaled to match a point.
     * The widget should not care much about this value, since the
     * transformation matrix will match the window scaling.
     */
    [[nodiscard]] float window_scale() const noexcept;

    /*! Called when the GPU library has changed the window size.
     */
    virtual void window_changed_size(extent2 new_extent);

    /*! Teardown Window based on State::*_LOST
     */
    virtual void teardown() = 0;

    /*! Build Windows based on State::NO_*
     */
    virtual void build() = 0;

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

    /*! Mouse mouse event.
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
    /** Target of the mouse
     * Since any mouse event will change the target this is used
     * to check if the target has changed, to send exit events to the previous mouse target.
     */
    std::weak_ptr<tt::widget> _mouse_target_widget = {};

    /** Target of the keyboard
     * widget where keyboard events are sent to.
     */
    std::weak_ptr<tt::widget> _keyboard_target_widget = {};

    /** Send event to a target widget.
     *
     * The commands are send in order, until the command is handled, then processing stops immediately.
     * All commands are tried in a batch to the following handlers:
     *  - The target widget
     *  - The parents of the widget up to and including the root widget.
     *  - The window itself.
     */
    template<typename Event>
    bool send_event_to_widget(std::shared_ptr<tt::widget> target_widget, Event const &event) noexcept;
};

} // namespace tt
