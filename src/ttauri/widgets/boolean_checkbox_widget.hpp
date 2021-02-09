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
    template<typename Value = observable<bool>>
    boolean_checkbox_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent, Value &&value = {}) noexcept :
        checkbox_widget<bool>(window, parent, true, false, std::forward<Value>(value))
    {
    }
};

}
