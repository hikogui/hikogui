// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "checkbox_widget.hpp"

namespace tt {

/** An abstract toggle button widget.
 * This widgets toggles a value from the true_value to false_value and back.
 */
class boolean_checkbox_widget : public checkbox_widget<bool> {
public:
    using super = checkbox_widget<bool>;
    using value_type = typename super::value_type;
    using delegate_type = typename super::delegate_type;

    template<typename Value = value_type>
    boolean_checkbox_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>(),
        Value &&value = value_type{}) noexcept :
        super(window, std::move(parent), std::move(delegate), std::forward<Value>(value))
    {
    }

    template<typename Value>
    boolean_checkbox_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        Value &&value) noexcept :
        boolean_checkbox_widget(window, std::move(parent), std::make_shared<delegate_type>(), std::forward<Value>(value))
    {
    }
};

}
