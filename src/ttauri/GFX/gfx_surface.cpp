// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_surface.hpp"
#include "gfx_device.hpp"
#include "gfx_system.hpp"
#include "../assert.hpp"

namespace tt::inline v1 {

void gfx_surface::set_device(gfx_device *new_device) noexcept
{
    tt_axiom(gfx_system_mutex.recurse_lock_count());

    if (_device == new_device) {
        return;
    }

    if (new_device) {
        // The assigned device must be from the same GUI-system.
        tt_assert(&system == &new_device->system);
    }

    if (_device) {
        state = gfx_surface_state::device_lost;
        teardown();
    }

    _device = new_device;
}

} // namespace tt::inline v1
