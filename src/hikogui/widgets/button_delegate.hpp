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

    virtual void set_state(widget_intf const& sender, widget_value value) noexcept {}

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

template<typename...>
class default_button_delegate;

template<>
class default_button_delegate<> : public button_delegate {
public:
    default_button_delegate() noexcept : button_delegate() {}

    /** Called when the button is pressed by the user.
     */
    void activate(widget_intf const& sender) noexcept override
    {
        _notifier();
    }
};

template<typename F, typename... Args>
requires std::invocable<F, Args...>
class default_button_delegate<F, Args...> : public button_delegate {
public:
    default_button_delegate(F&& function, Args&&... args) noexcept :
        button_delegate(), _function(std::forward<F>(function)), _args(std::forward<Args>(args)...)
    {
    }

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] widget_value state(widget_intf const& sender) const noexcept override
    {
        return _running ? widget_value::on : widget_value::off;
    }

    /** Called when the button is pressed by the user.
     *
     * Calls the function with the arguments.
     */
    void activate(widget_intf const& sender) noexcept override
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

} // namespace v1
} // namespace hi::v1
