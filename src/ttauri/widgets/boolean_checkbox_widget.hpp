// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "checkbox_widget.hpp"

namespace tt {

/** An abstract toggle button widget.
 * This widgets toggles a value from the true_value to false_value and back.
 */
class boolean_checkbox_widget : public checkbox_widget<bool> {
public:
    template<typename Value = observable<bool>>
    boolean_checkbox_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value = {}) noexcept :
        checkbox_widget<bool>(window, parent, true, false, std::forward<Value>(value))
    {
    }
};

}
