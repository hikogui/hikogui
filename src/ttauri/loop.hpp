// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "wfree_fifo.hpp"
#include "cast.hpp"
#include "net/network_event.hpp"
#include "GUI/gui_window.hpp"
#include <functional>
#include <type_traits>
#include <concepts>
#include <vector>
#include <memory>

namespace tt::inline v1 {

class loop {
public:
    struct impl_type {
    };

    /** Construct a loop.
     *
     */
    loop();

    ~loop();

    /** Create or get the main-loop.
     */
    [[nodiscard]] tt_no_inline static loop& main() noexcept;

    /** Set maximum frame rate.
     *
     * A frame rate above 30.0 may will cause the vsync thread to block on
     *
     * @param frame_rate The maximum frame rate that a window will be updated.
     */
    void set_maximum_frame_rate(double frame_rate) noexcept;

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
     * latency of network processing will be increased based on the amount of times
     * this function is called.
     *
     * @note This function must be called from the same thread as `resume()`.
     * @param block Allow processing to block, this is normally done only inside `resume()`.
     */
    void resume_once(bool block = false) noexcept;

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
    void add_socket(int fd, network_event event_mask, std::function<void(int, network_events const&)> f);

    /** Remove the callback associated with a socket.
     *
     * @param fd The file descriptor of the socket.
     */
    bool remove_socket(int fd);

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
        tt_axiom(is_same_thread());
        using async_task = async_task_type<Function, Args...>;

        auto& task = _async_fifo.emplace<async_task>(std::forward<Function>(f), std::forward<Args>(args)...);
        trigger_async();
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
        network_event mode;
        std::function<void(int, network_events const &)> callback;
    };

    /** Pointer to the main-loop.
     */
    inline static std::unique_ptr<loop> _main;

    std::unique_ptr<impl_type> _pimpl;

    wfree_fifo<async_task_base_type, 128> _async_fifo;
    std::optional<int> _exit_code;
    bool _is_main = false;
    double _maximum_frame_rate = 30.0;
    std::chrono::nanoseconds _minimum_frame_time = std::chrono::nanoseconds(33'333'333);
    thread_id _thread_id = 0;

    /** Get the private operating-system specific data.
     */
    template<typename T>
    T const& get_impl() const noexcept
    {
        tt_axiom(_pimpl);
        return down_cast<T const&>(*_pimpl);
    }

    /** Get the private operating-system specific data.
     */
    template<typename T>
    T& get_impl() noexcept
    {
        tt_axiom(_pimpl);
        return down_cast<T&>(*_pimpl);
    }

    /** Interrupt a blocking main loop to process newly added events.
     */
    void trigger_async() noexcept;

    /** Call elapsed timers.
     *
     * @param deadline The deadline before all timers must be handled before moving on.
     */
    void handle_vsync() noexcept;

    /** Handle all async calls.
     *
     * @param deadline The deadline before all calls must be executed before moving on.
     */
    void handle_async() noexcept;

    /** Handle the gui events.
     *
     * @param deadline The deadline before all gui-events are handled before moving on.
     */
    void handle_gui_events() noexcept;

    /** Check if the current thread is the loop's thread.
     */
    [[nodiscard]] bool is_same_thread() const noexcept
    {
        return current_thread_id() == _thread_id;
    }
};

} // namespace tt::inline v1
