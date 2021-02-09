// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_toggle_button_widget.hpp"

namespace tt {

/** An abstract boolean toggle button widget.
 * This widgets toggles a value from true to false and back.
 */
class abstract_bool_toggle_button_widget : public abstract_toggle_button_widget<bool> {
public:
    template<typename Value>
    abstract_bool_toggle_button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        Value &&value) noexcept :
        abstract_toggle_button_widget<bool>(window, parent, true, false, std::forward<Value>(value))
    {
    }
};

}