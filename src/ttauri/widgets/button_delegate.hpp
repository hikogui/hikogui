// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "button_state.hpp"
#include <memory>
#include <functional>

namespace tt {
class abstract_button_widget;

class button_delegate {
public:
    using callback_ptr_type = std::shared_ptr<std::function<void()>>;

    virtual ~button_delegate() {}
    button_delegate() noexcept {}
    button_delegate(button_delegate const &) = delete;
    button_delegate(button_delegate &&) = delete;
    button_delegate &operator=(button_delegate const &) = delete;
    button_delegate &operator=(button_delegate &&) = delete;

    virtual void init(abstract_button_widget &sender) noexcept {}

    virtual void deinit(abstract_button_widget &sender) noexcept {}

    /** Subscribe a callback for notifying the widget of a data change.
     */
    virtual callback_ptr_type subscribe(abstract_button_widget &sender, callback_ptr_type const &callback) noexcept
    {
        return callback;
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    template<typename Callback>
    requires(std::is_invocable_v<Callback>) [[nodiscard]] callback_ptr_type
        subscribe(abstract_button_widget &sender, Callback &&callback) noexcept
    {
        return subscribe(sender, std::make_shared<std::function<void()>>(std::forward<Callback>(callback)));
    }

    /** Unsubscribe a callback.
     */
    virtual void unsubscribe(abstract_button_widget &sender, callback_ptr_type const &callback) noexcept {}

    /** Called when the button is pressed by the user.
     */
    virtual void activate(abstract_button_widget &sender) noexcept {};

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] virtual button_state state(abstract_button_widget const &sender) const noexcept
    {
        return button_state::off;
    }
};

} // namespace tt
