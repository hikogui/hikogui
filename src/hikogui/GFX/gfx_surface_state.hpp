// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

enum class gfx_surface_state {
    no_window, ///< The window was destroyed, the device will drop the window on the next render cycle.
    has_window, ///< The initial state: The surface is associated with a window
    has_device, ///< The surface has been associated with a device to use for rendering.
    has_swapchain, ///< Images to render on to display on the surface are created.
};

enum class gfx_surface_loss {
    none, ///< Everything is running normally.
    swapchain_lost, ///< The window was resized, the swapchain needs to be rebuild and can not be rendered on.
    device_lost, ///< The device was lost, but the window could move to a new device, or the device can be recreated.
    window_lost, ///< The surface or window was destroyed, need to cleanup.
};

}
