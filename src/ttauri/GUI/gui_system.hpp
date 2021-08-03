// Copyright Take Vos 2020, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window.hpp"
#include "gui_window_win32.hpp"
#include "gui_system_delegate.hpp"
#include "vertical_sync.hpp"
#include "../GFX/gfx_system.hpp"
#include "../GFX/gfx_device.hpp"
#include "../thread.hpp"
#include "../unfair_recursive_mutex.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace tt {

/** Graphics system
 */
class gui_system {
public:
    static inline os_handle instance;

    std::unique_ptr<gfx_system> gfx;
    std::unique_ptr<vertical_sync> vsync;

    thread_id const thread_id;

    virtual ~gui_system();

    gui_system(const gui_system &) = delete;
    gui_system &operator=(const gui_system &) = delete;
    gui_system(gui_system &&) = delete;
    gui_system &operator=(gui_system &&) = delete;

    /** Initialize after construction.
     * Call this function directly after the constructor on the same thread.
     */
    virtual void init() noexcept
    {
        if (auto delegate = _delegate.lock()) {
            delegate->init(*this);
        }
    }

    virtual void deinit() noexcept
    {
        if (auto delegate = _delegate.lock()) {
            delegate->deinit(*this);
        }
    }

    void set_delegate(std::weak_ptr<gui_system_delegate> delegate) noexcept
    {
        _delegate = std::move(delegate);
    }

    /** Start the GUI event loop.
     *
     * This function will start the GUI event loop.
     * The event loop will monitor keyboard & mouse event, changes in window
     * size & position and rendering of all windows.
     *
     * When all windows are closed this function will return with an exit
     * code of zero, or the return value from the delegate.
     * Calling `exit()` will also cause this function to return.
     *
     * @return exit code.
     */
    virtual int loop() = 0;

    virtual void exit(int exit_code) = 0;

    /** Run the function from the GUI's event queue.
     */
    virtual void run_from_event_queue(std::function<void()> function) = 0;

    /** Run the function now or on from the GUI's event loop.
     */
    void run(std::function<void()> function) noexcept
    {
        if (is_gui_thread()) {
            function();
        } else {
            run_from_event_queue(std::move(function));
        }
    }

    gui_window &add_window(std::unique_ptr<gui_window> window);

    /** Create a new window.
     * @param args The arguments that are forwarded to the constructor of
     *             `tt::gui_window_win32`.
     * @return A reference to the new window.
     */
    template<typename... Args>
    gui_window &make_window(Args &&...args)
    {
        tt_axiom(is_gui_thread());

        // XXX abstract away the _win32 part.
        auto window = std::make_unique<gui_window_win32>(*this, std::forward<Args>(args)...);
        window->init();

        return add_window(std::move(window));
    }

    /*! Count the number of windows managed by the GUI.
     */
    ssize_t num_windows();

    void render(hires_utc_clock::time_point display_time_point)
    {
        tt_axiom(is_gui_thread());

        for (auto &window : _windows) {
            window->render(display_time_point);
            if (window->is_closed()) {
                window->deinit();
                window = nullptr;
            }
        }
        std::erase(_windows, nullptr);

        ttlet num_windows = std::size(_windows);
        if (num_windows == 0 && num_windows != _previous_num_windows) {
            // If last_window_closed() creates a new window we should
            // let it do that before entering the event queue again.
            // win32 is a bit picky about running without windows.
            if (auto delegate = _delegate.lock()) {
                if (auto exit_code = delegate->last_window_closed(*this)) {
                    this->exit(*exit_code);
                }
            } else {
                this->exit(0);
            }
        }
        _previous_num_windows = num_windows;
    }

    /** Check if this thread is the same as the gui thread.
     */
    [[nodiscard]] bool is_gui_thread() const noexcept
    {
        return thread_id == current_thread_id();
    }

    /** Make a gui_system instance.
     *
     * This will instantiate a gui_system instance appropriate for the current
     * operating system.
     * 
     * @return A unique pointer to a gui_system instance.
     */
    [[nodiscard]] static std::unique_ptr<gui_system> make_unique() noexcept;

    /** Set the theme for the system.
     *
     * @param new_theme The new theme to use for the gui system.
     */
    void set_theme(tt::theme *new_theme) noexcept
    {
        _theme = new_theme;
    }

    /** Get the theme set for the window.
     *
     * @return The current theme of the window, or the system if not set.
     */
    tt::theme const theme() const noexcept
    {
        tt_axiom(_theme);
        return _theme;
    }

protected:
    gui_system(std::unique_ptr<gfx_system> gfx, std::unique_ptr<vertical_sync> vsync) noexcept;

private:
    std::weak_ptr<gui_system_delegate> _delegate;

    std::vector<std::unique_ptr<gui_window>> _windows;
    size_t _previous_num_windows = 0;

    /** The theme of the system.
     * Should never be nullptr in reality.
     */
    tt::theme const *_theme = nullptr;
};

} // namespace tt
