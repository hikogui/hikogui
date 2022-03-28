// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "wfree_fifo.hpp"
#include <functional>
#include <type_traits>
#include <concepts>
#include <vector>

namespace tt::inline v1 {

class loop {
public:
    enum class select_type { none = 0, error = 0b001, read = 0b010, write = 0b100 };

    loop();

    /** Resume the loop on the current thread.
     *
     * @return Exit code when the loop is exited.
     */
    int resume() noexcept;

    /** Resume for a single iteration.
     *
     * The `resume_once(false)` may be used to continue processing events and
     * GUI redraws while the GUI event queue is blocked. This happens on win32 when
     * a window is being moved, resized, the title bar or system menu being clicked.
     *
     * It should be called often, as it will be used to process network messages and
     * latency of network processing will be increased based on the ammount of times
     * this function is called.
     *
     * @note This function must be called from the same thread as `resume()`.
     * @param block Allow processing to block, this is normally done only inside `resume()`.
     */
    void resume_once(bool block = false) noexcept;

    /** The time when the next timed event will happen.
     *
     * Timed events are:
     * - Callbacks added with `add_timer()`,
     * - window redraws.
     *
     * This function may be used together with `resume_once()` to determine
     * when to call it.
     */
    utc_nanoseconds wake_time() const noexcept;

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
    void add_socket(int fd, select_type mode, std::invocable<int, select_type> auto&& f)
    {
        tt_axiom(fd >= 0);
        if (fd > max_fd()) {
            throw io_error(
                std::format("Socket descriptor {} is higher than the maximum {} supported value by select()", fd, max_fd()));
        }

        _sockets.emplace_back(fd, mode, tt_forward(f));
    }

    /** Remove the callback associated with a socket.
     *
     * @param fd The file descriptor of the socket.
     */
    bool remove_socket(int fd);

    /** Add a callback to be called at a specific time.
     *
     * @note The callback will be removed from the loop when it is called.
     * @param wake_time The time at which the call the callback
     * @param f The callback to call.
     */
    template<typename Function>
    void add_timer(utc_nanoseconds wake_time, std::invocable<> auto&& f)
    {
        // Insert earlier wake_times at the end of the vector.
        auto it = std::lower_bound(_timers.begin(), _timers.end(), wake_time, [](ttlet& item, ttlet& value) {
            return item.wake_time > value;
        });

        _timers.emplace(it, wake_time, tt_forward(f));
    }

    /** Call a function from the loop.
     *
     * @note It is safe to call this function from another thread.
     * @note If the function and arguments uses a small amount of storage space and the fifo is not full,
     *       this function is wait-free.
     * @param f The function to call from the event-loop
     * @param args The arguments to pass to the function.
     * @return A future for the return value.
     */
    template<typename Function, typename... Args>
    [[nodiscard]] auto async(Function&& f, Args&&...args) noexcept
    {
        using async_task = async_task_type<Function, Args...>;

        auto& task = _async_fifo.emplace<async_task>(std::forward<Function>(f), std::forward<Args>(args)...);
        interrupt();
        return task.get_future();
    }

private:
    struct async_task_base_type {
        virtual void operator()() noexcept = 0;
    };

    template<typename Function, typename... Args>
    struct async_task_type : async_task_base_type {
        using result_type = std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>;
        using future_type = std::future<result_type>;
        using promise_type = std::promise<result_type>;

        async_task_type(Function&& f, Args&&...args) noexcept :
            async_task_base_type(), _function(std::forward<Function>(f)), _args(std::forward<Args>(args)...)
        {
        }

        void operator()() noexcept override
        {
            try {
                _promise.set_value(std::apply(std::move(_function), std::move(_args)));
            } catch (...) {
                _promise.set_exception(std::current_exception());
            }
        }

        future_type get_future() noexcept
        {
            return _promise.get_future();
        }

        Function _function;
        std::tuple<std::decay_t<Args>...> _args;
        promise_type _promise;
    };

    struct socket_type {
        int fd;
        select_type mode;
        std::function<void(int, select_type)> callback;
    };

    struct timer_type {
        utc_nanoseconds wake_time;
        std::function<void()> callback;
    };

    wfree_fifo<async_task_base_type, 128> _async_fifo;
    std::vector<socket_type> _sockets;
    std::vector<timer_type> _timers;
    std::optional<int> _exit_code;

    /** Interrupt a blocking main loop to process newly added events.
     */
    void interrupt() noexcept;

    /** Block on network events.
     *
     * This function will block on network sockets, and potentially run callbacks
     * associated with these network sockets.
     *
     * @param deadline The deadline before all network events must be handled before moving on.
     */
    void block_on_network(utc_nanoseconds deadline) noexcept;

    /** Handle the redraw of windows.
    * 
     * @param deadline The deadline before redraw must be finished before moving on.
     */
    void handle_redraw(utc_nanoseconds deadline) noexcept;

    /** Call elapsed timers.
    * 
     * @param deadline The deadline before all timers must be handled before moving on.
     */
    void handle_timers(utc_nanoseconds deadline) noexcept;

    /** Handle all async calls.
     *
     * @param deadline The deadline before all calls must be executed before moving on.
     */
    void handle_async(utc_nanoseconds deadline) noexcept;

    /** Handle the gui events.
     *
     * @param deadline The deadline before all gui-events are handled before moving on.
     */
    void handle_gui_events(utc_nanoseconds deadline) noexcept;

    /** Maximum socket value supported.
     */
    int max_fd() const noexcept;
};

[[nodiscard]] loop::select_type operator&(loop::select_type const& lhs, loop::select_type const& rhs) noexcept
{
    return static_cast<loop::select_type>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] loop::select_type operator|(loop::select_type const& lhs, loop::select_type const& rhs) noexcept
{
    return static_cast<loop::select_type>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] loop::select_type& operator|=(loop::select_type& lhs, loop::select_type const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

[[nodiscard]] bool any(loop::select_type const& rhs) noexcept
{
    return static_cast<bool>(to_underlying(rhs));
}

} // namespace tt::inline v1
