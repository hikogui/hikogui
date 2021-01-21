// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

namespace tt {

enum class gui_window_state {
    initializing, ///< The window has not been initialized yet.
    no_window, ///< The window was destroyed, the device will drop the window on the next render cycle.
    no_device, ///< No device is associated with the Window and can therefor not be rendered on.
    no_surface, ///< Need to request a new surface before building a swapchain
    no_swapchain, /// Need to request a swapchain before rendering.
    ready_to_render, ///< The swapchain is ready drawing is allowed.
    swapchain_lost, ///< The window was resized, the swapchain needs to be rebuild and can not be rendered on.
    surface_lost, ///< The Vulkan surface on the window was destroyed.
    device_lost, ///< The device was lost, but the window could move to a new device, or the device can be recreated.
    window_lost, ///< The window was destroyed, need to cleanup.
};

}