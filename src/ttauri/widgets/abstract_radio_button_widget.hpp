// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"

namespace tt {

/** An abstract radio button widget.
 * This widgets set the value to true_value when pressed.
 */
template<typename T>
class abstract_radio_button_widget : public abstract_button_widget<T> {
public:
    using super = abstract_button_widget<T>;
    using value_type = T;

    template<typename Value = observable<value_type>>
    abstract_radio_button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        value_type true_value,
        Value &&value = {}) noexcept :
        super(window, parent, std::move(true_value), std::forward<Value>(value))
    {
        _value_callback = this->value.subscribe([this](auto...) {
            ttlet lock = std::scoped_lock(gui_system_mutex);
            this->request_redraw();
        });
        _callback = this->subscribe([this]() {
            this->select();
        });
    }

    /** Select this radio-button.
     */
    void select() noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (compare_then_assign(this->value, this->true_value)) {
            this->request_redraw();
        }
    }

private:
    typename decltype(super::value)::callback_ptr_type _value_callback;
    typename super::callback_ptr_type _callback;
};

} // namespace tt