// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "button_state.hpp"
#include <memory>
#include <functional>

namespace hi::inline v1 {
class abstract_button_widget;

class button_delegate {
public:
    virtual ~button_delegate() = default;

    virtual void init(abstract_button_widget &sender) noexcept {}

    virtual void deinit(abstract_button_widget &sender) noexcept {}

    /** Subscribe a callback for notifying the widget of a data change.
     */
    [[nodiscard]] auto subscribe(abstract_button_widget &sender, std::invocable<> auto &&callback) noexcept
    {
        return _notifier.subscribe(hi_forward(callback));
    }

    /** Called when the button is pressed by the user.
     */
    virtual void activate(abstract_button_widget &sender) noexcept {};

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] virtual button_state state(abstract_button_widget const &sender) const noexcept
    {
        return button_state::off;
    }

protected:
    notifier<> _notifier;
};

} // namespace hi::inline v1
