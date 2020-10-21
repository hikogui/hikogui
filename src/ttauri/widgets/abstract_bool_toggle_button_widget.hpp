// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_toggle_button_widget.hpp"

namespace tt {

/** An abstract toggle button widget.
 * This widgets toggles a value from the true_value to false_value and back.
 */
class abstract_bool_toggle_button_widget : public abstract_toggle_button_widget<bool> {
public:
    template<typename Arg>
    abstract_bool_toggle_button_widget(Window &window, Widget *parent, Arg &&arg) noexcept :
        abstract_toggle_button_widget<bool>(window, parent, true, false, std::forward<Arg>(arg)) {}
};

}