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
    [[nodiscard]] virtual cancel_features_type features() const noexcept
    {
        return cancel_features_type::none;
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

/** A default async button delegate.
 *
 * The default async button delegate manages the state of a button widget using
 * observer values.
 *
 * @ingroup widget_delegates
 * @tparam Traits The traits of the arguments passed to the constructor.
 */
template<typename ResultType = void>
class default_async_delegate : public async_delegate {
public:
    using result_type = ResultType;

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
    default_async_delegate(Func&& func, Args&&... args) noexcept :
        _task_controller(std::forward<Func>(func), std::forward<Args>(args)...)
    {
        _task_controller_cbt = _task_controller.subscribe([this] {
            this->_notifier();
        });
    }

    /// @privatesection
    [[nodiscard]] widget_value state(widget_intf const& sender) const noexcept override
    {
        if (not _task_controller.runnable()) {
            return widget_value::other;

        } else if (_task_controller.running()) {
            return widget_value::on;

        } else {
            return widget_value::off;
        }
    }

    [[nodiscard]] cancel_features_type features() const noexcept override
    {
        return _task_controller.features();
    }

    void activate(widget_intf const& sender) noexcept override
    {
        if (not _task_controller.runnable()) {
            return;

        } else if (not _task_controller.running()) {
            _task_controller.run();

        } else if (
            _task_controller.features() == cancel_features_type::stop or
            _task_controller.features() == cancel_features_type::stop_and_progress) {
            _task_controller.request_stop();
        }
    }
    /// @endprivatesection
private:
    task_controller<result_type> _task_controller;
    task_controller<result_type>::callback_type _task_controller_cbt;
};

template<typename FuncType, typename... ArgTypes>
using default_async_delegate_result_type =
    invoke_task_result_t<decltype(cancelable_async_task<FuncType, ArgTypes...>), FuncType, std::stop_token, progress_token, ArgTypes...>;

template<typename F, typename... Args>
default_async_delegate(F&& func, Args&&... args) -> default_async_delegate<default_async_delegate_result_type<F, Args...>>;

} // namespace v1
}
