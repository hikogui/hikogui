// Copyright Take Vos 2020, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window.hpp"
#include "gui_window_win32.hpp"
#include "gui_system_delegate.hpp"
#include "../text/unicode_bidi_class.hpp"
#include "../GFX/gfx_device.hpp"
#include "../thread.hpp"
#include "../unfair_recursive_mutex.hpp"
#include "../event_queue.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace tt::inline v1 {
class gfx_system;
class vertical_sync;
class font_book;
class theme_book;
class keyboard_bindings;

/** Graphics system
 */
class gui_system {
public:
    static inline os_handle instance;

    std::unique_ptr<gfx_system> gfx;
    std::unique_ptr<tt::vertical_sync> vertical_sync;
    std::unique_ptr<tt::font_book> font_book;
    std::unique_ptr<tt::theme_book> theme_book;
    std::unique_ptr<tt::keyboard_bindings> keyboard_bindings;

    thread_id const thread_id;

    /** The writing direction.
     *
     * The writing direction determines the initial writing direction of paragraphs.
     *
     * When the value is `R` the user-interface should flip the horizontal layout,
     * for example a row-layout-widget in flipped-mode should layout children ordered from
     * right to left.
     *
     * @note The only values allowed are `L` and `R`.
     */
    unicode_bidi_class writing_direction = unicode_bidi_class::L;

    /** Make a gui_system instance.
     *
     * This will instantiate a gui_system instance appropriate for the current
     * operating system.
     *
     * @param delegate An optional delegate.
     * @return A unique pointer to a gui_system instance.
     */
    [[nodiscard]] static std::unique_ptr<gui_system> make_unique(std::weak_ptr<gui_system_delegate> delegate = {}) noexcept;

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

    /** Get the event queue.
     *
     * This queue allows for adding jobs to the queue which will
     * be executed on the gui thread.
     */
    tt::event_queue const &event_queue() const noexcept
    {
        return *_event_queue;
    }

    /** Run the function from the GUI's event queue.
     */
    void run_from_event_queue(std::invocable auto &&function) noexcept
    {
        event_queue().emplace(std::forward<decltype(function)>(function));
    }

    /** Run the function now or on from the GUI's event loop.
     */
    void run(std::invocable auto &&function) noexcept
    {
        if (is_gui_thread()) {
            function();
        } else {
            run_from_event_queue(std::forward<decltype(function)>(function));
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

    void render(utc_nanoseconds display_time_point)
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

        ttlet num_windows = size(_windows);
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

    /** Set the theme for the system.
     *
     * @param new_theme The new theme to use for the gui system.
     */
    void set_theme(tt::theme const &new_theme) noexcept;

    /** Get the theme.
     *
     * @return The current theme.
     */
    tt::theme const &theme() const noexcept;

    void set_theme_mode(tt::theme_mode mode) noexcept;

    /** Request all windows to constrain.
     */
    void request_reconstrain() noexcept;

protected:
    gui_system(
        std::shared_ptr<tt::event_queue> event_queue,
        std::unique_ptr<gfx_system> gfx,
        std::unique_ptr<tt::vertical_sync> vertical_sync,
        std::unique_ptr<tt::font_book> font_book,
        std::unique_ptr<tt::theme_book> theme_book,
        std::unique_ptr<tt::keyboard_bindings> keyboard_bindings,
        std::weak_ptr<gui_system_delegate> delegate = {}) noexcept;

    /** The event queue to invoke events on the gui thread.
     *
     * The event queue is a shared_ptr to allow the event queue to be allocated
     * in locked memory. To ensure non-blocking emplace().
     */
    std::shared_ptr<tt::event_queue> _event_queue;

private:
    std::weak_ptr<gui_system_delegate> _delegate;

    std::vector<std::unique_ptr<gui_window>> _windows;
    std::size_t _previous_num_windows = 0;

    /** The theme of the system.
     * Should never be nullptr in reality.
     */
    tt::theme const *_theme = nullptr;
};

} // namespace tt::inline v1
