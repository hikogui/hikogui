// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "function_timer.hpp"
#include "socket_event.hpp"
#include "../container/module.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <functional>
#include <type_traits>
#include <concepts>
#include <vector>
#include <memory>

hi_export_module(hikogui.dispatch.loop : intf);

namespace hi::inline v1 {

class loop {
public:
    using timer_callback_token = function_timer<>::callback_token;
    using render_callback_type = std::function<void(utc_nanoseconds)>;
    using render_callback_token = std::shared_ptr<render_callback_type>;

    class impl_type {
    public:
        bool is_main = false;

        impl_type() : _thread_id(current_thread_id()) {}

        virtual ~impl_type() {}
        impl_type(impl_type const&) = delete;
        impl_type(impl_type&&) = delete;
        impl_type& operator=(impl_type const&) = delete;
        impl_type& operator=(impl_type&&) = delete;

        virtual void set_maximum_frame_rate(double frame_rate) noexcept = 0;
        virtual void set_vsync_monitor_id(uintptr_t id) noexcept = 0;
        
        void wfree_post_function(auto&& func) noexcept
        {
            return _function_fifo.add_function(hi_forward(func));
        }

        void post_function(auto&& func) noexcept
        {
            _function_fifo.add_function(hi_forward(func));
            notify_has_send();
        }

        [[nodiscard]] auto async_function(auto&& func) noexcept
        {
            auto future = _function_fifo.add_async_function(hi_forward(func));
            notify_has_send();
            return future;
        }

        timer_callback_token delay_function(utc_nanoseconds time_point, auto&& func) noexcept
        {
            auto [token, first_to_call] = _function_timer.delay_function(time_point, hi_forward(func));
            if (first_to_call) {
                // Notify if the added function is the next function to call.
                notify_has_send();
            }
            return token;
        }

        timer_callback_token repeat_function(std::chrono::nanoseconds period, utc_nanoseconds time_point, auto&& func) noexcept
        {
            auto [token, first_to_call] = _function_timer.repeat_function(period, time_point, hi_forward(func));
            if (first_to_call) {
                // Notify if the added function is the next function to call.
                notify_has_send();
            }
            return token;
        }

        timer_callback_token repeat_function(std::chrono::nanoseconds period, auto&& func) noexcept
        {
            auto [token, first_to_call] = _function_timer.repeat_function(period, hi_forward(func));
            if (first_to_call) {
                // Notify if the added function is the next function to call.
                notify_has_send();
            }
            return token;
        }

        virtual void subscribe_render(std::weak_ptr<render_callback_type> f) noexcept = 0;
        virtual void add_socket(int fd, socket_event event_mask, std::function<void(int, socket_events const&)> f) = 0;
        virtual void remove_socket(int fd) = 0;
        virtual int resume(std::stop_token stop_token) noexcept = 0;
        virtual void resume_once(bool block) noexcept = 0;

        [[nodiscard]] bool on_thread() const noexcept
        {
            return current_thread_id() == _thread_id;
        }

    protected:
        /** Notify the event loop that a function was added to the _function_fifo.
         */
        virtual void notify_has_send() noexcept = 0;

        function_fifo<> _function_fifo;
        function_timer<> _function_timer;

        std::optional<int> _exit_code = {};
        double _maximum_frame_rate = 30.0;
        std::chrono::nanoseconds _minimum_frame_time = std::chrono::nanoseconds(33'333'333);
        thread_id _thread_id;
        std::vector<std::weak_ptr<render_callback_type>> _render_functions;
    };

    /** Construct a loop.
     *
     */
    loop();
    loop(loop const&) = delete;
    loop(loop&&) noexcept = default;
    loop& operator=(loop const&) = delete;
    loop& operator=(loop&&) noexcept = default;

    /** Get or create the thread-local loop.
     */
    [[nodiscard]] hi_no_inline static loop& local() noexcept
    {
        if (not _local) {
            _local = std::make_unique<loop>();
        }
        return *_local;
    }

    /** Get or create the main-loop.
     *
     * @note The first time main() is called must be from the main-thread.
     *       In this case there is no race condition on the first time main() is called.
     */
    [[nodiscard]] hi_no_inline static loop& main() noexcept
    {
        if (auto ptr = _main.load(std::memory_order::acquire)) {
            return *ptr;
        }

        hi_axiom(_timer.load(std::memory_order::relaxed) == nullptr, "loop::main() must be called before loop::timer()");

        // This is the first time loop::main() is called so we must be on the main-thread
        // So name the thread "main" so we can find it during debugging.
        set_thread_name("main");

        auto ptr = std::addressof(local());
        ptr->_pimpl->is_main = true;
        _main.store(ptr, std::memory_order::release);
        return *ptr;
    }

    /** Get or create the timer event-loop.
     *
     * @note The first time this is called a thread is started to handle the timer events.
     */
    [[nodiscard]] hi_no_inline static loop& timer() noexcept
    {
        // The first time timer() is called, make sure that the main-loop exists,
        // or even create the main-loop on the current thread.
        [[maybe_unused]] hilet &tmp = loop::main();

        return *start_subsystem_or_terminate(_timer, nullptr, timer_init, timer_deinit);
    }

    /** Set maximum frame rate.
     *
     * A frame rate above 30.0 may will cause the vsync thread to block on
     *
     * @param frame_rate The maximum frame rate that a window will be updated.
     */
    void set_maximum_frame_rate(double frame_rate) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->set_maximum_frame_rate(frame_rate);
    }

    /** Set the monitor id for vertical sync.
     */
    void set_vsync_monitor_id(uintptr_t id) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->set_vsync_monitor_id(id);
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
    void wfree_post_function(auto&& func) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->wfree_post_function(hi_forward(func));
    }

    /** Post a function to be called from the loop.
     *
     * @note It is safe to call this function from another thread.
     * @param func The function to call from the loop. The function must not take any arguments and return void.
     */
    void post_function(auto&& func) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->post_function(hi_forward(func));
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
        hi_assert_not_null(_pimpl);
        return _pimpl->async_function(hi_forward(func));
    }

    /** Call a function at a certain time.
     *
     * @param time_point The time at which to call the function.
     * @param func The function to be called.
     */
    [[nodiscard]] timer_callback_token delay_function(utc_nanoseconds time_point, auto&& func) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->delay_function(time_point, hi_forward(func));
    }

    /** Call a function repeatedly.
     *
     * @param period The period between calls to the function.
     * @param time_point The time at which to call the function.
     * @param func The function to be called.
     */
    [[nodiscard]] timer_callback_token
    repeat_function(std::chrono::nanoseconds period, utc_nanoseconds time_point, auto&& func) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->repeat_function(period, time_point, hi_forward(func));
    }

    /** Call a function repeatedly.
     *
     * @param period The period between calls to the function.
     * @param func The function to be called.
     */
    [[nodiscard]] timer_callback_token repeat_function(std::chrono::nanoseconds period, auto&& func) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->repeat_function(period, hi_forward(func));
    }

    /** Subscribe a render function to be called on vsync.
     *
     * @param f A weak pointer to a function to be called when vsync occurs.
     */
    void subscribe_render(std::weak_ptr<render_callback_type> f) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->subscribe_render(std::move(f));
    }

    /** Subscribe a render function to be called on vsync.
     *
     * @param f A function to be called when vsync occurs.
     */
    template<std::invocable<utc_nanoseconds> F>
    render_callback_token subscribe_render(F &&f) noexcept
    {
        hi_assert_not_null(_pimpl);
        auto ptr = std::make_shared<render_callback_type>(std::forward<F>(f));
        _pimpl->subscribe_render(ptr);
        return ptr;
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
     * @param event_mask The socket events to wait for.
     * @param f The callback to call when the file descriptor unblocks.
     */
    void add_socket(int fd, socket_event event_mask, std::function<void(int, socket_events const&)> f)
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->add_socket(fd, event_mask, std::move(f));
    }

    /** Remove the callback associated with a socket.
     *
     * @param fd The file descriptor of the socket.
     */
    void remove_socket(int fd)
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->remove_socket(fd);
    }

    /** Resume the loop on the current thread.
     *
     * @param stop_token The thread's stop token to use to determine when to stop.
     *                   If not stop token is given, then resume will automatically stop when there
     *                   are no more windows, sockets, functions or timers.
     * @return Exit code when the loop is exited.
     */
    int resume(std::stop_token stop_token = {}) noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->resume(stop_token);
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
        hi_assert_not_null(_pimpl);
        return _pimpl->resume_once(block);
    }

    /** Check if the current thread is the same as the loop's thread.
     *
     * The loop's thread is the thread that calls resume().
     */
    [[nodiscard]] bool on_thread() const noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->on_thread();
    }

private:
    static loop *timer_init() noexcept
    {
        hi_assert(not _timer_thread.joinable());

        _timer_thread = std::jthread{[](std::stop_token stop_token) {
            _timer.store(std::addressof(loop::local()), std::memory_order::release);

            set_thread_name("timer");
            loop::local().resume(stop_token);
        }};

        while (true) {
            if (auto ptr = _timer.load(std::memory_order::relaxed)) {
                return ptr;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    static void timer_deinit() noexcept
    {
        if (auto const *const ptr = _timer.exchange(nullptr, std::memory_order::acquire)) {
            hi_assert(_timer_thread.joinable());
            _timer_thread.request_stop();
            _timer_thread.join();
        }
    }

    inline static thread_local std::unique_ptr<loop> _local;

    /** Pointer to the main-loop.
     */
    inline static std::atomic<loop *> _main;

    /** Pointer to the timer-loop.
     */
    inline static std::atomic<loop *> _timer;

    inline static std::jthread _timer_thread;

    std::unique_ptr<impl_type> _pimpl;
};

template<typename Result, typename... Args>
template<typename F>
void notifier<Result(Args...)>::loop_local_post_function(F&& f) const noexcept
{
    return loop::local().post_function(std::forward<F>(f));
}

template<typename Result, typename... Args>
template<typename F>
void notifier<Result(Args...)>::loop_main_post_function(F&& f) const noexcept
{
    return loop::main().post_function(std::forward<F>(f));
}

template<typename Result, typename... Args>
template<typename F>
void notifier<Result(Args...)>::loop_timer_post_function(F&& f) const noexcept
{
    return loop::timer().post_function(std::forward<F>(f));
}

} // namespace hi::inline v1
