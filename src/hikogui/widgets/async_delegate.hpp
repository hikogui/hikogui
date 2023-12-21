// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/async_delegate.hpp Defines async_delegate and some default async delegates.
 * @ingroup widget_delegates
 */

#pragma once

#include "../observer/observer.hpp"
#include "../utility/utility.hpp"
#include "../concurrency/concurrency.hpp"
#include "../dispatch/dispatch.hpp"
#include "../GUI/GUI.hpp"
#include "../macros.hpp"
#include <type_traits>
#include <memory>

hi_export_module(hikogui.widgets.async_delegate);

hi_export namespace hi {
inline namespace v1 {

/** A button delegate controls the state of a button widget.
 * @ingroup widget_delegates
 */
class async_delegate {
public:
    virtual ~async_delegate() = default;

    virtual void init(widget_intf const& sender) noexcept {}

    virtual void deinit(widget_intf const& sender) noexcept {}

    /** Called when the button is pressed by the user.
     */
    virtual void activate(widget_intf const& sender) noexcept {}

    /** Used by the widget to determine if it can stop.
     */
    [[nodiscard]] virtual bool can_stop() const noexcept
    {
        return false;
    }

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] virtual widget_value state(widget_intf const& sender) const noexcept
    {
        return widget_value::off;
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    template<forward_of<void()> Func>
    [[nodiscard]] callback<void()> subscribe(Func&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(std::forward<Func>(func), flags);
    }

protected:
    notifier<void()> _notifier;
};

template<typename T>
struct default_async_delegate_result_traits {
    constexpr static bool is_task = false;
    using type = std::decay_t<T>;
};

template<typename T>
struct default_async_delegate_result_traits<::hi::task<T>> {
    constexpr static bool is_task = true;
    using type = std::decay_t<T>;
};

template<typename F, typename... Args>
struct default_async_delegate_traits {
    static_assert(
        std::is_invocable_v<F, Args...> or std::is_invocable_v<F, Args..., std::stop_token>,
        "Incorrect arguments for function.");

    /** The callable must be called with a std::stop_token as last argument.
     */
    constexpr static bool uses_stop_token = std::is_invocable_v<F, Args..., std::stop_token>;

    /** The result type of the callable.
     */
    using result_type =
        std::conditional<uses_stop_token, std::invoke_result_t<F, Args..., std::stop_token>, std::invoke_result_t<F, Args...>>;

    /** The callable is a task (co-routine).
     */
    constexpr static bool is_task = default_async_delegate_result_traits<result_type>::is_task;

    /** The value returned by the callable, after stripping the optional async-wrapper.
     */
    using value_type = default_async_delegate_result_traits<result_type>::type;

    using task_type = task<value_type>;

    using function_type =
        std::conditional<uses_stop_token, std::function<task_type(Args..., std::stop_token)>, std::function<task_type(Args...)>>;

    template<typename... A>
    [[nodiscard]] constexpr static std::tuple<std::decay_t<Args>...> make_tuple(A&&...a) noexcept
    {
        return std::tuple<std::decay_t<Args>...>{std::forward<A>(a)...};
    }
};

/** A default async button delegate.
 *
 * The default async button delegate manages the state of a button widget using
 * observer values.
 *
 * @ingroup widget_delegates
 * @tparam Traits The traits of the arguments passed to the constructor.
 */
template<typename Traits>
class default_async_delegate : public async_delegate {
public:
    using value_type = Traits::value_type;

    /** Construct a delegate.
     *
     * @note The function may accept a std::stop_token as a last argument,
     *       this stop-token is passed automatically when the button is pressed
     *       and the stop_token must not be passed as an argument to this
     *       constructor.
     * @param func The function to be called when the button is pressed
     * @param args... The arguments passed to the function
     */
    template<typename Func, typename... Args>
    default_async_delegate(Func&& func, Args&&...args) noexcept
    {
        auto tmp_arguments = Traits::argument_types{std::forward<Args>(args)...};

        auto tmp_function = [&] {
            if constexpr (Traits::is_task) {
                return std::forward<Func>(func);
            } else {
                return make_async_task<Func, Args...>(std::forward<Func>(func));
            }
        }();

        _function = [function = std::move(tmp_function),
                     arguments = std::move(tmp_arguments)](std::stop_token stop_token) -> task<value_type> {
            if constexpr (Traits::uses_stop_token) {
                return std::apply(function, std::tuple_cat(arguments, std::tuple{stop_token}));
            } else {
                return std::apply(function, arguments);
            }
        };
    }

    /// @privatesection
    [[nodiscard]] widget_value state(widget_intf const& sender) const noexcept override
    {
        return _task.running() ? widget_value::on : widget_value::off
    }

    [[nodiscard]] bool can_stop() const noexcept override
    {
        return Traits::uses_stop_token;
    }

    void activate(widget_intf const& sender) noexcept override
    {
        if (_task.done() or not _task.started()) {
            if constexpr (Traits::uses_stop_token) {
                _stop_source = {};

                _task = _function(_stop_source.get_token());

                _task_cbt = _task.subscribe([&] {
                    // Notify the widget when the task is done.
                    _notifier();
                });

                // Notify the widget that the task has started.
                _notifier();

            } else {
                _stop_source.request_stop();
            }
        }
    }
    
    /// @endprivatesection
private:
    std::function<task<value_type>(std::stop_token)> _function;
    task<value_type> _task;
    std::stop_source _stop_source;
};

template<typename F, typename... Args>
default_async_delegate(F&& func, Args&&...args)
    -> default_async_delegate<default_async_delegate_traits<F, Args...>>;

}}
