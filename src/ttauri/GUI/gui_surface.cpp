// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

namespace tt {

#include "gui_surface.hpp"
#include "gui_device.hpp"
#include "gui_system.hpp"
#include "../assert.hpp"

void gui_surface::set_device(gui_device *new_device) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (_device == new_device) {
        return;
    }

    if (new_device) {
        // The assigned device must be from the same GUI-system.
        tt_assert(&system == &new_device->system);
    }

    if (_device) {
        state = gui_surface_state::device_lost;
        teardown();
    }

    _device = new_device;
}

}
