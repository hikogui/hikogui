// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"

namespace tt {

/** An abstract toggle button widget.
 * This widgets toggles a value from the true_value to false_value and back.
 */
template<typename T>
class abstract_toggle_button_widget : public abstract_button_widget {
public:
    using value_type = T;

    value_type const true_value;
    value_type const false_value;
    observable<value_type> value;

    template<typename Arg = observable<value_type>>
    abstract_toggle_button_widget(
        Window &window,
        std::shared_ptr<widget> parent,
        value_type true_value,
        value_type false_value,
        Arg &&arg = {}) noexcept :
        abstract_button_widget(window, parent),
        true_value(std::move(true_value)),
        false_value(std::move(false_value)),
        value(std::forward<Arg>(arg))
    {
        _value_callback = this->value.subscribe([this](auto...) {
            this->window.requestRedraw = true;
        });
        _callback = subscribe([this]() {
            this->toggle();
        });
    }

    void toggle() noexcept
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);

        if (compare_then_assign(value, value == false_value ? true_value : false_value)) {
            window.requestRedraw = true;
        }
    }

private:
    typename decltype(value)::callback_ptr_type _value_callback;
    typename abstract_button_widget::callback_ptr_type _callback;
};

} // namespace tt