// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "abstract_button_widget.hpp"
#include "../GUI/gui_system.hpp"

namespace tt {

void abstract_button_widget::activate() noexcept
{
    _delegate->activate(*this);

    gui_system::global().run_from_event_queue([this]() {
        this->_notifier();
    });
}

}