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

    observable<value_type> value;
    value_type const true_value;
    value_type const false_value;

    template<typename Arg>
    abstract_toggle_button_widget(
        Window &window,
        Widget *parent,
        value_type true_value,
        value_type false_value,
        Arg &&arg) noexcept :
        abstract_button_widget(window, parent),
        value(std::forward<Arg>(arg)),
        true_value(std::move(true_value)),
        false_value(std::move(false_value))
    {
        _callback = {*this, [this]() { this->toggle(); }};
    }

    void toggle() noexcept {
        ttlet lock = std::scoped_lock(mutex);
    
        if (compare_then_assign(value, value == false_value ? true_value : false_value)) {
            window.requestRedraw = true;
        }
    }

private:
    scoped_callback<abstract_button_widget> _callback;
};

} // namespace tt