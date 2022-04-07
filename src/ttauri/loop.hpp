// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "function_fifo.hpp"
#include "function_timer.hpp"
#include "cast.hpp"
#include "subsystem.hpp"
#include "net/network_event.hpp"
#include "GUI/gui_window.hpp"
#include <functional>
#include <type_traits>
#include <concepts>
#include <vector>
#include <memory>

namespace tt::inline v1 {
class gui_window;

class loop {
public:
    class impl_type {
    public:
        impl_type() : _thread_id(0) {}

        virtual ~impl_type() {}

        virtual void set_maximum_frame_rate(double frame_rate) noexcept = 0;

        [[nodiscard]] void wfree_post_function(auto&& func) noexcept
        {
            return _function_fifo.add_function(tt_forward(func));
        }

        [[nodiscard]] void post_function(auto&& func) noexcept
        {
            _function_fifo.add_function(tt_forward(func));
            notify_has_send();
        }

        [[nodiscard]] auto async_function(auto&& func) noexcept
        {
            auto future = _function_fifo.add_async_function(tt_forward(func));
            notify_has_send();
            return future;
        }

        void delay_function(utc_nanoseconds time_point, auto&& func) noexcept
        {
            if (_function_timer.delay_function(time_point, tt_forward(func))) {
                // Notify if the added function is the next function to call.
                notify_has_send();
            }
        }

        void repeat_function(std::chrono::nanoseconds period, utc_nanoseconds time_point, auto&& func) noexcept
        {
            if (_function_timer.repeat_function(period, time_point, tt_forward(func))) {
                // Notify if the added function is the next function to call.
                notify_has_send();
            }
        }

        void repeat_function(std::chrono::nanoseconds period, auto&& func) noexcept
        {
            if (_function_timer.repeat_function(period, tt_forward(func))) {
                // Notify if the added function is the next function to call.
                notify_has_send();
            }
        }

        virtual void add_window(std::weak_ptr<gui_window> window) noexcept = 0;
        virtual void add_socket(int fd, network_event event_mask, std::function<void(int, network_events const&)> f) = 0;
        virtual void remove_socket(int fd) = 0;
        virtual int resume() noexcept = 0;
        virtual void resume_once(bool block) noexcept = 0;

    protected:
        /** Notify the event loop that a function was added to the _function_fifo.
         */
        virtual void notify_has_send() noexcept = 0;

        [[nodiscard]] bool is_same_thread() const noexcept
        {
            return _thread_id == 0 or current_thread_id() == _thread_id;
        }

        function_fifo<> _function_fifo;
        function_timer<> _function_timer;

        std::optional<int> _exit_code;
        bool _is_main = false;
        double _maximum_frame_rate = 30.0;
        std::chrono::nanoseconds _minimum_frame_time = std::chrono::nanoseconds(33'333'333);
        thread_id _thread_id = 0;
        std::vector<std::weak_ptr<gui_window>> _windows;
    };

    /** Construct a loop.
     *
     */
    loop();
    loop(loop const&) = delete;
    loop(loop&&) noexcept = default;
    loop& operator=(loop const&) = delete;
    loop& operator=(loop&&) noexcept = default;

    /** Create or get the main-loop.
     */
    [[nodiscard]] tt_no_inline static loop& main() noexcept
    {
        return *start_subsystem_or_terminate(_main, nullptr, subsystem_init, subsystem_deinit);
    }

    /** Set maximum frame rate.
     *
     * A frame rate above 30.0 may will cause the vsync thread to block on
     *
     * @param frame_rate The maximum frame rate that a window will be updated.
     */
    void set_maximum_frame_rate(double frame_rate) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->set_maximum_frame_rate(frame_rate);
    }

    /** Wait-free post a function to be called from the loop.
     *
     * @note It is safe to call this function from another thread.
     * @note The event loop is not directly notified that a new function exists
     *       and will be delayed until after the loop has woken for other work.
     * @note The post is only wait-free if the function fifo is not full,
     *       and the function is small enough to fit in a slot on the fifo.
     * @param func The function to call from the loop. The function must not take any arguments and return void.
     */
    [[nodiscard]] void wfree_post_function(auto&& func) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->wfree_post_function(tt_forward(func));
    }

    /** Post a function to be called from the loop.
     *
     * @note It is safe to call this function from another thread.
     * @param func The function to call from the loop. The function must not take any arguments and return void.
     */
    [[nodiscard]] void post_function(auto&& func) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->post_function(tt_forward(func));
    }

    /** Call a function from the loop.
     *
     * @note It is safe to call this function from another thread.
     * @param func The function to call from the loop. The function must not take any argument,
     *             but may return a value.
     * @return A `std::future` for the return value.
     */
    [[nodiscard]] auto async_function(auto&& func) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->async_function(tt_forward(func));
    }

    /** Call a function at a certain time.
    * 
    * @param time_point The time at which to call the function.
    * @param func The function to be called.
    */
    [[nodiscard]] void delay_function(utc_nanoseconds time_point, auto&& func) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->delay_function(time_point, tt_forward(func));
    }

    /** Call a function repeatedly.
     *
     * @param period The period between calls to the function.
     * @param time_point The time at which to call the function.
     * @param func The function to be called.
     */
    [[nodiscard]] void repeat_function(std::chrono::nanoseconds period, utc_nanoseconds time_point, auto&& func) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->repeat_function(period, time_point, tt_forward(func));
    }

    /** Call a function repeatedly.
     *
     * @param period The period between calls to the function.
     * @param func The function to be called.
     */
    [[nodiscard]] void repeat_function(std::chrono::nanoseconds period, auto&& func) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->repeat_function(period, tt_forward(func));
    }

    /** Add a window to be redrawn from the event loop.
     *
     * @param window A reference to an existing window, ready to be redrawn.
     */
    void add_window(std::weak_ptr<gui_window> window) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->add_window(std::move(window));
    }

    /** Add a callback that reacts on a socket.
     *
     * In most cases @a mode is set to one of the following values:
     * - error | read: Unblock when there is data available for read.
     * - error | write: Unblock when there is buffer space available for write.
     * - error | read | write: Unblock when there is data available for read of when there is buffer space available for write.
     *
     * @note Only one callback can be associated with a socket.
     * @param fd File descriptor of the socket.
     * @param mode The mode of how select should work with the socket.
     * @param f The callback to call when the file descriptor unblocks.
     */
    void add_socket(int fd, network_event event_mask, std::function<void(int, network_events const&)> f)
    {
        tt_axiom(_pimpl);
        return _pimpl->add_socket(fd, event_mask, std::move(f));
    }

    /** Remove the callback associated with a socket.
     *
     * @param fd The file descriptor of the socket.
     */
    void remove_socket(int fd)
    {
        tt_axiom(_pimpl);
        return _pimpl->remove_socket(fd);
    }

    /** Resume the loop on the current thread.
     *
     * @return Exit code when the loop is exited.
     */
    int resume() noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->resume();
    }

    /** Resume for a single iteration.
     *
     * The `resume_once(false)` may be used to continue processing events and
     * GUI redraws while the GUI event queue is blocked. This happens on win32 when
     * a window is being moved, resized, the title bar or system menu being clicked.
     *
     * It should be called often, as it will be used to process network messages and
     * latency of network processing will be increased based on the amount of times
     * this function is called.
     *
     * @note This function must be called from the same thread as `resume()`.
     * @param block Allow processing to block, this is normally done only inside `resume()`.
     */
    void resume_once(bool block = false) noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->resume_once(block);
    }

private:
    /** Pointer to the main-loop.
     */
    inline static std::atomic<loop *> _main;

    std::unique_ptr<impl_type> _pimpl;

    static loop *subsystem_init() noexcept
    {
        return new loop();
    }

    static void subsystem_deinit() noexcept
    {
        if (auto tmp = _main.exchange(nullptr)) {
            delete tmp;
        }
    }
};

} // namespace tt::inline v1
