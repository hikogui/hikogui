// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/button_delegate.hpp Defines button_delegate and some default button delegates.
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

hi_export_module(hikogui.widgets.button_delegate);

hi_export namespace hi {
inline namespace v1 {

/** A button delegate controls the state of a button widget.
 * @ingroup widget_delegates
 */
class button_delegate {
public:
    virtual ~button_delegate() = default;

    virtual void init(widget_intf const& sender) noexcept {}

    virtual void deinit(widget_intf const& sender) noexcept {}

    /** Called when the button is pressed by the user.
     */
    virtual void activate(widget_intf const& sender) noexcept {}

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

/** A default radio button delegate.
 *
 * The default radio button delegate manages the state of a button widget using
 * observer values.
 *
 * @ingroup widget_delegates
 * @tparam T The type of the observer value.
 */
template<std::equality_comparable T>
class default_radio_button_delegate : public button_delegate {
public:
    using value_type = T;

    observer<value_type> value;
    observer<value_type> on_value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     * @param on_value The value or observer-value that mean 'on'.
     */
    default_radio_button_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<value_type>> auto&& on_value) noexcept :
        value(hi_forward(value)), on_value(hi_forward(on_value))
    {
        // clang-format off
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _on_value_cbt = this->on_value.subscribe([&](auto...){ this->_notifier(); });
        // clang-format on
    }

    /// @privatesection
    [[nodiscard]] widget_value state(widget_intf const& sender) const noexcept override
    {
        if (*value == *on_value) {
            return widget_value::on;
        } else {
            return widget_value::off;
        }
    }

    void activate(widget_intf const& sender) noexcept override
    {
        value = *on_value;
    }
    /// @endprivatesection
private:
    callback<void(value_type)> _value_cbt;
    callback<void(value_type)> _on_value_cbt;
};

template<typename Value, typename OnValue>
default_radio_button_delegate(Value&&, OnValue&&) -> default_radio_button_delegate<observer_decay_t<Value>>;

/** A default toggle button delegate.
 *
 * The default toggle button delegate manages the state of a button widget using
 * observer values.
 *
 * @ingroup widget_delegates
 * @tparam T The type of the observer value.
 */
template<std::equality_comparable T>
class default_toggle_button_delegate : public button_delegate {
public:
    using value_type = T;
    constexpr static bool can_make_defaults =
        std::is_same_v<value_type, bool> or std::is_integral_v<value_type> or std::is_enum_v<value_type>;

    observer<value_type> value;
    observer<value_type> on_value;
    observer<value_type> off_value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     * @param on_value The value or observer-value that mean 'on'.
     * @param off_value The value or observer-value that mean 'off'.
     */
    default_toggle_button_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<value_type>> auto&& on_value,
        forward_of<observer<value_type>> auto&& off_value) noexcept :
        value(hi_forward(value)), on_value(hi_forward(on_value)), off_value(hi_forward(off_value))
    {
        // clang-format off
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _on_value_cbt = this->on_value.subscribe([&](auto...){ this->_notifier(); });
        _off_value_cbt = this->off_value.subscribe([&](auto...){ this->_notifier(); });
        // clang-format on
    }

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     * @param on_value The value or observer-value that mean 'on'.
     */
    default_toggle_button_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<value_type>> auto&& on_value) noexcept
        requires can_make_defaults
        : default_toggle_button_delegate(hi_forward(value), hi_forward(on_value), value_type{})
    {
    }

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    default_toggle_button_delegate(forward_of<observer<value_type>> auto&& value) noexcept
        requires can_make_defaults
        : default_toggle_button_delegate(hi_forward(value), value_type{1}, value_type{})
    {
    }

    /// @privatesection
    [[nodiscard]] widget_value state(widget_intf const& sender) const noexcept override
    {
        if (*value == *on_value) {
            return widget_value::on;
        } else if (*value == *off_value) {
            return widget_value::off;
        } else {
            return widget_value::other;
        }
    }

    void activate(widget_intf const& sender) noexcept override
    {
        if (*value == *off_value) {
            value = *on_value;
        } else {
            value = *off_value;
        }
    }
    /// @endprivatesection
private:
    callback<void(value_type)> _value_cbt;
    callback<void(value_type)> _on_value_cbt;
    callback<void(value_type)> _off_value_cbt;
};

template<typename Value>
default_toggle_button_delegate(Value&&) -> default_toggle_button_delegate<observer_decay_t<Value>>;

template<typename Value, typename OnValue>
default_toggle_button_delegate(Value&&, OnValue&&) -> default_toggle_button_delegate<observer_decay_t<Value>>;

template<typename Value, typename OnValue, typename OffValue>
default_toggle_button_delegate(Value&&, OnValue&&, OffValue&&) -> default_toggle_button_delegate<observer_decay_t<Value>>;

/** Make a shared pointer to a radio-button delegate.
 *
 * @ingroup widget_delegates
 * @see default_radio_button_delegate
 * @param value A value or observer-value used as a representation of the state.
 * @param on_value The value or observer-value that mean 'on'.
 * @return A shared_ptr to a button delegate.
 */
[[nodiscard]] std::shared_ptr<button_delegate> make_default_radio_button_delegate(auto&& value, auto&& on_value) noexcept
    requires requires {
        default_radio_button_delegate<observer_decay_t<decltype(value)>>{hi_forward(value), hi_forward(on_value)};
    }
{
    return std::make_shared<default_radio_button_delegate<observer_decay_t<decltype(value)>>>(
        hi_forward(value), hi_forward(on_value));
}

/** Make a shared pointer to a toggle-button delegate.
 *
 * @ingroup widget_delegates
 * @see default_toggle_button_delegate
 * @param value A value or observer-value used as a representation of the state.
 * @param args an optional on-value followed by an optional off-value.
 * @return A shared_ptr to a button delegate.
 */
[[nodiscard]] std::shared_ptr<button_delegate> make_default_toggle_button_delegate(auto&& value, auto&&...args) noexcept
    requires requires {
        default_toggle_button_delegate<observer_decay_t<decltype(value)>>{hi_forward(value), hi_forward(args)...};
    }
{
    return std::make_shared<default_toggle_button_delegate<observer_decay_t<decltype(value)>>>(
        hi_forward(value), hi_forward(args)...);
}

template<typename T>
struct default_async_button_delegate_result_traits {
    constexpr auto bool is_task = false;
    using type = std::decay_t<T>;
};

template<typename T>
struct default_async_button_delegate_result_traits<::hi::task<T>> {
    constexpr auto bool is_task = true;
    using type = std::decay_t<T>;
};

template<typename F, typename... Args>
struct default_async_button_delegate_traits {
    static_assert(
        std::is_invocable_v<F, Args...> or std::is_invocable_v<F, Args..., std::stop_token>,
        "Incorrect arguments for function.");

    /** The callable must be called with a std::stop_token as last argument.
     */
    constexpr auto bool uses_stop_token = std::is_invocable_v<F, Args..., std::stop_token>;

    /** The result type of the callable.
     */
    using result_type =
        std::conditional<uses_stop_token, std::invoke_result_t<F, Args..., std::stop_token>, std::invoke_result_t<F, Args...>>;

    /** The value returned by the callable, after stripping the optional async-wrapper.
     */
    using value_type = default_async_button_delegate_result_traits<result_type>::type;

    /** The callable is a task (co-routine).
     */
    constexpr auto bool is_task = default_async_button_delegate_result_traits<result_type>::is_task;

    using task_type = task<value_type>;

    using function_type =
        std::conditional<uses_stop_token, std::function<task_type(Args..., std::stop_token)>, std::function<task_type(Args...)>>;

    template<typename... A>
    [[nodiscard]] constexpr static std::tuple<std::decay_t<Args>...> make_tuple(A &&...a) noexcept
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
class default_async_button_delegate : public button_delegate {
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
    default_async_button_delegate(Func&& func, Args&&...args) noexcept
    {
        _function = [this, function = std::forward<Func>(func), arguments = Traits::make_tuple(std::forward<Args>(args)...)](
                    std::stop_token stop_token) -> task<value_type> {
            if constexpr (Traits::is_task) {
                if constexpr (Traits::uses_stop_token) {
                    return std::apply(function, std::tuple_cat(this->_arguments, std::make_tuple(stop_token)));
                } else {
                    return std::apply(function, this->_arguments);
                }
            } else {
                if constexpr (Traits::uses_stop_token) {
                    return std::apply(
                        async_task, std::tuple_cat(std::make_tuple(function), this->_arguments, std::make_tuple(stop_token)));
                } else {
                    return std::apply(async_task, std::tuple_cat(std::make_tuple(function), this->_arguments));
                }
            }
        };
    }

    /// @privatesection
    [[nodiscard]] widget_value state(widget_intf const& sender) const noexcept override
    {
        return _task.running() ? widget_value::on : widget_value::off
    }

    [[nodiscard]] constexpr bool can_stop() const noexcept
    {
        return Traits::uses_stop_token;
    }

    void activate(widget_intf const& sender) noexcept override
    {
        if (_task.done() or not _task.started()) {
            _stop_source = {};

            _task = _function(_stop_source.get_token());

            _task_cbt = _task.subscribe([&] {
                // Notify the widget when the task is done.
                _notifier();
            });

        } else {
            _stop_source.request_stop();
        }
    }
    /// @endprivatesection
private:
    std::function<task<value_type>(std::stop_token)> _function;
    task<value_type> _task;
    std::stop_source _stop_source;
};

template<typename F, typename... Args>
default_async_button_delegate(F&& func, Args&&...args)
    -> default_async_button_delegate<default_async_button_delegate_traits<F, Args...>>;

} // namespace v1
} // namespace hi::v1
