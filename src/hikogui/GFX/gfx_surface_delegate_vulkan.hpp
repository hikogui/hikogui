// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_surface_delegate.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <cstdint>

namespace hi::inline v1 {

class gfx_surface_delegate_vulkan : public gfx_surface_delegate {
public:
    /** The vulkan device is going to be teared-down.
     */
    virtual void device_teardown() noexcept = 0;

    /** The vulkan device has been initialized.
     *
     * This function is called when either the device has just been build, or when the widget
     * is added to a window with the device already existing.
     *
     * The device may be rebuild when the vulkan device disconnects.
     */
    virtual void device_setup(vk::Instance instance, vk::Device device, vk::Queue graphics_queue) noexcept = 0;

    /** The swap-chain is going to be teared-down.
     *
     * This function is called just before the swap-chain is being teared down.
     *
     * This requires the destruction of any references to the swap-chain's image views, including
     * the frame-buffers created during `swapchain_build()`.
     */
    virtual void swapchain_teardown() noexcept = 0;

    /** The swap-chain has been build.
     *
     * This function is called when either the swap-chain has just been build, or when the widget
     * is added to a window with the swap-chain already existing.
     *
     * The swap-chain will also be build during resizing of the window. So this needs to be rather fast.
     *
     * @param views The list of swap-chain image views.
     * @param size The size of the images in the swap-chain.
     * @param format The pixel format of the images in the swap-chain.
     */
    virtual void swapchain_setup(std::vector<vk::ImageView> views, vk::Extent2d size, vk::Format format) noexcept = 0;

    /** Draw using vulkan API.
     *
     * @param swapchain_index The index of the image-view of the swap-chain to draw into.
     * @param clipping_rectangle The rectangle on the window that is visible through this widget.
     * @param render_area The area of the window that is being drawn.
     * @param start The semaphore used to signal when the @a image_view is ready to be drawn.
     * @param finish The semaphore used to signal when the HikoGUI overlay is drawn onto the @a image_view.
     */
    virtual void draw_vulkan(
        uint32_t swapchain_index,
        aarectangle clipping_rectangle,
        aarectangle render_area,
        vk::Semaphore start,
        vk::Semaphore finish) noexcept = 0;
};

} // namespace hi::inline v1
