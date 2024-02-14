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

    [[nodiscard]] constexpr virtual invocable_features_type features() const noexcept = 0;

    virtual task<result_type> run(std::stop_token stop_token, hi::progress_token progress_token) = 0;
};

template<typename FuncType, typename... ArgTypes>
class task_controller_impl : public task_controller_base<std::invoke_result_t<FuncType, ArgTypes...>> {
public:
    using result_type = std::invoke_result_t<FuncType, ArgTypes...>;

    template<typename Func, typename... Args>
    task_controller_impl(Func&& func, Args&&... args) : _func(std::forward<Func>(func)), _args(std::forward<Args>(args))
    {
    }

    [[nodiscard]] constexpr invocable_features_type features() const noexcept override
    {
        return invocable_features_v<FuncType, ArgTypes...>;
    }

    task<result_type> run(std::stop_token stop_token, hi::progress_token progress_token)
    {
        return std::apply(
            cancelable_async_task<FuncType, ArgTypes...>,
            std::tuple_cat(std::tuple{std::move(stop_token), std::move(progress_token)}, _args));
    }

private:
    FuncType _func;
    std::tuple<ArgTypes...> _args;
};

} // namespace detail

class task_running_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

template<typename ResultType>
class task_controller {
public:
    using result_type = ResultType;

    task_controller(task_controller const&) = delete;
    task_controller(task_controller&&) = delete;
    task_controller& operator=(task_controller const&) = delete;
    task_controller& operator=(task_controller&&) = delete;

    ~task_controller()
    {
        unset_function();
    }

    task_controller() noexcept
    {
        _progress_cb = _progress_sink.subscribe([this] {
            this->_notifier();
        });
    }

    template<typename Func, typename... Args>
    task_controller(Func&& func, Args&&... args) :
        task_controller(),
        _pimpl(std::make_shared<detail::task_controller_impl<std::decay_t<Func>, std::decay_t<Args>...>>(
            std::forward<Func>(func),
            std::forward<Args>(args)...))
    {
    }

    template<typename Func, typename... Args>
    void set_function(Func&& func, Args&&... args)
    {
        reset();
        _pimpl = std::make_shared<detail::task_controller_impl<std::decay_t<Func>, std::decay_t<Args>...>>(
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

    [[nodiscard]] invocable_features_type features() const noexcept
    {
        if (_pimpl) {
            return _pimpl->features();
        } else {
            return invocable_features_type::none;
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

    [[nodiscard]] void run()
    {
        reset();
        _task = _pimpl->run(_stop_source.get_token(), _progress_sink.get_token());
    }

    void request_stop() noexcept
    {
        return _stop_source.request_stop();
    }

    [[nodiscard]] float_t progress() const noexcept
    {
        return _progress_sink.value();
    }

    [[nodiscard]] result_type value() const requires not std::same_as<result_type, void>
    {
        return _task.value();
    }

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
