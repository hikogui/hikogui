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

    virtual void init(widget_intf const* sender) {}

    virtual void deinit(widget_intf const* sender) {}

    /** Called when the button is pressed by the user.
     */
    virtual void activate(widget_intf const* sender) {}

    virtual void set_state(widget_intf const* sender, widget_value value) {}

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] virtual widget_value state(widget_intf const* sender) const
    {
        return widget_value::off;
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    template<forward_of<void()> Func>
    [[nodiscard]] callback<void()>
    subscribe(widget_intf const* sender, Func&& func, callback_flags flags = callback_flags::synchronous)
    {
        return _notifier.subscribe(std::forward<Func>(func), flags);
    }

protected:
    notifier<void()> _notifier;
};

template<typename...>
class default_button_delegate;

template<>
class default_button_delegate<> : public button_delegate {
public:
    default_button_delegate() noexcept : button_delegate() {}

    /** Called when the button is pressed by the user.
     */
    void activate(widget_intf const* sender) noexcept override
    {
        _notifier();
    }
};

/** Button delegate handling a task with a stop_token.
 */
template<typename F, typename... Args>
requires invocable_task<F, std::stop_token, Args...>
class default_button_delegate<F, std::stop_token, Args...> : public button_delegate {
public:
    default_button_delegate(F&& function, Args&&... args) :
        button_delegate(),
        _function(std::forward<F>(function)),
        _args(std::forward<Args>(args)...)
    {
    }

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] widget_value state(widget_intf const* sender) const override
    {
        if (_task.running()) {
            if (_stop_source.stop_possible() and _stop_source.stop_requested()) {
                return widget_value::other;
            } else {
                return widget_value::on;
            }
        } else {
            return widget_value::off;
        }
    }

    /** Called when the button is pressed by the user.
     *
     * Calls the function with the arguments.
     */
    void activate(widget_intf const* sender) override
    {
        assert(loop::main().on_thread());

        if (_task.running()) {
            assert(_stop_source.stop_possible());
            _stop_source.request_stop();

        } else {
            _stop_source = std::stop_source();

            _task = std::apply(_function, std::tuple_cat(std::tuple{_stop_source.get_token()}, _args));

            _task_cbt = _task.subscribe([this] {
                assert(_task.done());

                _task_cbt = {};
                _task = {};
                _notifier();
            });

            // Call the notifier to update the state of the button.
            _notifier();
        }
    }

private:
    F _function;
    std::tuple<Args...> _args;
    std::stop_source _stop_source;
    task<void> _task;
    callback<void()> _task_cbt;
};

/** Button delegate handling a task.
 */
template<typename F, typename... Args>
requires invocable_task<F, Args...>
class default_button_delegate<F, Args...> : public button_delegate {
public:
    default_button_delegate(F&& function, Args&&... args) :
        button_delegate(), _function(std::forward<F>(function)), _args(std::forward<Args>(args)...)
    {
    }

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] widget_value state(widget_intf const* sender) const override
    {
        return _task.running() ? widget_value::on : widget_value::off;
    }

    /** Called when the button is pressed by the user.
     *
     * Calls the function with the arguments.
     */
    void activate(widget_intf const* sender) override
    {
        assert(loop::main().on_thread());

        if (_task.running()) {
            return;
        }

        _task = std::apply(_function, _args);

        _task_cbt = _task.subscribe([this] {
            assert(_task.done());

            _task_cbt = {};
            _task = {};
            _notifier();
        });

        // Call the notifier to update the state of the button.
        _notifier();
    }

private:
    F _function;
    std::tuple<Args...> _args;
    task<void> _task;
    callback<void()> _task_cbt;
};

/** Button delegate handling a long-running function with std::stop_token argument..
 */
template<typename F, typename... Args>
requires std::invocable<F, std::stop_token, Args...>
class default_button_delegate<F, std::stop_token, Args...> : public button_delegate {
public:
    default_button_delegate(F&& function, Args&&... args) :
        button_delegate(), _function(std::forward<F>(function)), _args(std::forward<Args>(args)...)
    {
    }

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] widget_value state(widget_intf const* sender) const override
    {
        if (_running) {
            if (_stop_source.stop_requested()) {
                return widget_value::other;
            } else {
                return widget_value::on;
            }
        } else {
            return widget_value::off;
        }
    }

    /** Called when the button is pressed by the user.
     *
     * Calls the function with the arguments.
     */
    void activate(widget_intf const* sender) override
    {
        assert(loop::main().on_thread());

        if (_running) {
            assert(_stop_source.is_stoppable());
            _stop_source.request_stop();

        } else {
            _stop_source = {};
            _running = true;
            _future = std::async(std::launch::async, [this] {
                std::apply(_function, std::cat_tupple(std::tuple{_stop_source.get_token()}, _args));
            });
            _future_cbt = loop::local().delay_function_until(
                [this] {
                    assert(_future.valid());
                    return _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                },
                [this] {
                    assert(_future.valid());
                    _future.get();
                    _future = {};
                    _future_cbt = {};
                    _running = false;

                    _notifier();
                });

            // Call the notifier to update the state of the button.
            _notifier();
        }
    }

private:
    bool _running = false;
    F _function;
    std::tuple<Args...> _args;
    std::future<void> _future;
    callback<void()> _future_cbt;
    std::stop_source _stop_source;
};

/** Button delegate handling a long-running function.
 */
template<typename F, typename... Args>
requires std::invocable<F, Args...>
class default_button_delegate<F, Args...> : public button_delegate {
public:
    default_button_delegate(F&& function, Args&&... args) :
        button_delegate(), _function(std::forward<F>(function)), _args(std::forward<Args>(args)...)
    {
    }

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] widget_value state(widget_intf const* sender) const override
    {
        return _running ? widget_value::on : widget_value::off;
    }

    /** Called when the button is pressed by the user.
     *
     * Calls the function with the arguments.
     */
    void activate(widget_intf const* sender) override
    {
        assert(loop::main().on_thread());

        if (_running) {
            return;
        }

        _running = true;
        _future = std::async(std::launch::async, [this] {
            std::apply(_function, _args);
        });
        _future_cbt = loop::local().delay_function_until(
            [this] {
                assert(_future.valid());
                return _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            },
            [this] {
                _running = false;

                assert(_future.valid());
                _future.get();
                _notifier();
            });

        // Call the notifier to update the state of the button.
        _notifier();
    }

private:
    bool _running = false;
    F _function;
    std::tuple<Args...> _args;
    std::future<void> _future;
    callback<void()> _future_cbt;
};

default_button_delegate() -> default_button_delegate<>;

template<typename F, typename... Args>
requires std::invocable<F, Args...>
default_button_delegate(F&&, Args&&...) -> default_button_delegate<F, Args...>;

template<typename F, typename... Args>
requires std::invocable<F, std::stop_token, Args...>
default_button_delegate(F&&, Args&&...) -> default_button_delegate<F, std::stop_token, Args...>;


} // namespace v1
} // namespace hi::v1
