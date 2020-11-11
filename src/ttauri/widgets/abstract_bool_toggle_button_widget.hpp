// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_toggle_button_widget.hpp"

namespace tt {

/** An abstract boolean toggle button widget.
 * This widgets toggles a value from true to false and back.
 */
class abstract_bool_toggle_button_widget : public abstract_toggle_button_widget<bool> {
public:
    template<typename Value>
    abstract_bool_toggle_button_widget(Window_base &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        abstract_toggle_button_widget<bool>(window, parent, true, false, std::forward<Value>(value))
    {
    }
};

}