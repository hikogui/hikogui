// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"

namespace tt {

/** An abstract radio button widget.
 * This widgets set the value to true_value when pressed.
 */
template<typename T>
class abstract_radio_button_widget : public abstract_button_widget {
public:
    using value_type = T;

    value_type const true_value;
    observable<value_type> value;

    template<typename Arg = observable<value_type>>
    abstract_radio_button_widget(
        Window &window,
        std::shared_ptr<widget> parent,
        value_type true_value,
        Arg &&arg = {}) noexcept :
        abstract_button_widget(window, parent),
        true_value(std::move(true_value)),
        value(std::forward<Arg>(arg))
    {
        _value_callback = this->value.subscribe([this](auto...) {
            this->window.requestRedraw = true;
        });
        _callback = subscribe([this]() {
            this->select();
        });
    }

    /** Select this radio-button.
     */
    void select() noexcept
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);

        if (compare_then_assign(value, true_value)) {
            window.requestRedraw = true;
        }
    }

private:
    typename decltype(value)::callback_ptr_type _value_callback;
    typename abstract_button_widget::callback_ptr_type _callback;
};

} // namespace tt