// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "task.hpp"
#include "async_task.hpp"
#include "progress.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.dispatch.async_controller);

hi_export namespace hi {
inline namespace v1 {
namespace detail {

template<typename ResultType>
class task_controller_base {
public:
    using result_type = ResultType;

    virtual ~task_controller_base() = default;

    [[nodiscard]] constexpr virtual cancel_features_type features() const noexcept = 0;

    virtual task<result_type> run(std::stop_token stop_token, hi::progress_token progress_token) = 0;
};

template<typename ResultType, typename FuncType, typename... ArgTypes>
class task_controller_impl : public task_controller_base<ResultType> {
public:
    using result_type = ResultType;

    template<typename Func, typename... Args>
    task_controller_impl(Func&& func, Args&&... args) : _func(std::forward<Func>(func)), _args(std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] constexpr cancel_features_type features() const noexcept override
    {
        return cancel_features_v<FuncType, ArgTypes...>;
    }

    task<result_type> run(std::stop_token stop_token, hi::progress_token progress_token)
    {
        return std::apply(
            cancelable_async_task<FuncType, ArgTypes...>,
            std::tuple_cat(std::tuple{_func, std::move(stop_token), std::move(progress_token)}, _args));
    }

private:
    FuncType _func;
    std::tuple<ArgTypes...> _args;
};

} // namespace detail

class task_running_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/** A task controller.
 *
 * @tparam ResultType The type of the result of a hi::task or function.
 */
template<typename ResultType>
class task_controller {
public:
    /** The result type returned by a hi::task or function.
     */
    using result_type = ResultType;
    using callback_type = notifier<>::callback_type;

    task_controller(task_controller const&) = delete;
    task_controller(task_controller&&) = delete;
    task_controller& operator=(task_controller const&) = delete;
    task_controller& operator=(task_controller&&) = delete;

    ~task_controller()
    {
        unset_function();
    }

    /** Create a new task_controller.
     */
    task_controller() noexcept
    {
        _progress_cb = _progress_sink.subscribe([this] {
            this->_notifier();
        });
    }

    /** Create a new task_controller with a assigned coroutine or function and its arguments.
     *
     * @param func The coroutine or function to execute when run() is called.
     * @param args The arguments passed to the function when run() is called.
     */
    template<typename Func, typename... Args>
    task_controller(Func&& func, Args&&... args) requires compatible_cancelable_async_callable<ResultType, Func, Args...>
        : task_controller()
    {
        _pimpl = std::make_shared<detail::task_controller_impl<result_type, std::decay_t<Func>, std::decay_t<Args>...>>(
            std::forward<Func>(func), std::forward<Args>(args)...);
    }

    /** Set the coroutine or function and its arguments.
     *
     * @param func The coroutine or function to execute when run() is called.
     * @param args The arguments passed to the function when run() is called.
     * @throws hi::task_running_error when the task is currently running.
     */
    template<typename Func, typename... Args>
    void set_function(Func&& func, Args&&... args) requires compatible_cancelable_async_callable<ResultType, Func, Args...>
    {
        reset();
        _pimpl = std::make_shared<detail::task_controller_impl<result_type, std::decay_t<Func>, std::decay_t<Args>...>>(
            std::forward<Func>(func), std::forward<Args>(args)...);
    }

    /** Remove the task, so that it can no longer be run.
     *
     * @throws hi::task_running_error when the task is currently running.
     */
    void unset_function()
    {
        using namespace std::literals;

        reset();
        _pimpl = nullptr;
    }

    /** The features of the coroutine or function that was assigned.
     *
     * @return none, stop, progress or stop_and_progress.
     */
    [[nodiscard]] cancel_features_type features() const noexcept
    {
        if (_pimpl) {
            return _pimpl->features();
        } else {
            return cancel_features_type::none;
        }
    }

    /** Check if a function is assigned.
     */
    [[nodiscard]] bool runnable() const noexcept
    {
        return static_cast<bool>(_pimpl);
    }

    /** Check if the function was started.
     */
    [[nodiscard]] bool started() const noexcept
    {
        if (not runnable()) {
            return false;
        }
        return _task.started();
    }

    /** Check if the function is currently running.
     */
    [[nodiscard]] bool running() const noexcept
    {
        if (not runnable()) {
            return false;
        }
        return _task.running();
    }

    /** Check if the function has completed.
     */
    [[nodiscard]] bool done() const noexcept
    {
        if (not runnable()) {
            return false;
        }
        return _task.done();
    }

    /** Reset the state of the function to not-started.
     *
     * @throws hi::task_running_error when the function is currently running.
     */
    void reset()
    {
        if (running()) {
            throw task_running_error("Task is running.");
        }

        _task = {};
        _stop_source = {};
        _progress_sink.reset();
    }

    /** Run the assigned coroutine or function with the previous given arguments.
     */
    void run()
    {
        reset();
        _task = _pimpl->run(_stop_source.get_token(), _progress_sink.get_token());
    }

    /** Request stop.
     *
     * @pre The assigned coroutine or function must accept a std::stop_token.
     * @post The coroutine or function is requested to stop.
     */
    bool request_stop() noexcept
    {
        return _stop_source.request_stop();
    }

    /** Get progress of a function.
     *
     * @pre The assigned coroutine or function must accept a hi::progress_token.
     * @return The current progress as reported by the coroutine or function through the hi::progress_token.
     */
    [[nodiscard]] float_t progress() const noexcept
    {
        return _progress_sink.value();
    }

    /** Get the return value from the coroutine or function.
     *
     * @pre done() must return true.
     * @return The value returned by `return` or `co_return`.
     */
    [[nodiscard]] result_type value() const requires(not std::same_as<result_type, void>)
    {
        return _task.value();
    }

    /** Register a callback to be called when a coroutine or function reports progress.
     */
    template<typename Callback>
    hi::notifier<>::callback_type subscribe(Callback&& callback, callback_flags flags = callback_flags::synchronous)
    {
        return _notifier.subscribe(std::forward<Callback>(callback), flags);
    }

private:
    std::shared_ptr<detail::task_controller_base<result_type>> _pimpl = {};
    std::stop_source _stop_source = {};
    progress_sink _progress_sink = {};
    progress_sink::callback_type _progress_cb = {};
    task<result_type> _task = {};
    hi::notifier<> _notifier = {};
};

} // namespace v1
}
