// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/button_delegate.hpp Defines button_delegate and some default button delegates.
 * @ingroup widget_delegates
 */

#pragma once

#include "../notifier.hpp"
#include "../observer.hpp"
#include "../GUI/module.hpp"
#include <type_traits>
#include <memory>

namespace hi { inline namespace v1 {
class widget;

/** A button delegate controls the state of a button widget.
 * @ingroup widget_delegates
 */
class button_delegate {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    virtual ~button_delegate() = default;

    virtual void init(widget& sender) noexcept {}

    virtual void deinit(widget& sender) noexcept {}

    /** Called when the button is pressed by the user.
     */
    virtual void activate(widget& sender) noexcept {};

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] virtual widget_state state(widget const& sender) const noexcept
    {
        return widget_state::off;
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    [[nodiscard]] callback_token
    subscribe(forward_of<callback_proto> auto&& callback, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(hi_forward(callback), flags);
    }

protected:
    notifier_type _notifier;
};


}} // namespace hi::v1
