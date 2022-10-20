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
    /** The vulkan device has been initialized.
     *
     * This function is called when either the device has just been build, or when the widget
     * is added to a window with the device already existing.
     *
     * The device may be rebuild when the vulkan device disconnects.
     */
    virtual void build_for_new_device(
        VmaAllocator allocator,
        vk::Instance instance,
        vk::Device device,
        vk::Queue graphics_queue,
        uint32_t graphics_queue_family_index) noexcept = 0;

    /** The swap-chain has been build.
     *
     * This function is called when either the swap-chain has just been build, or when the widget
     * is added to a window with the swap-chain already existing.
     *
     * The swap-chain will also be build during resizing of the window. So this needs to be rather fast.
     *
     * @param views The list of swap-chain image views.
     * @param size The size of the images in the swap-chain.
     * @param format The pixel format and color space of the images in the swap-chain.
     */
    virtual void
    build_for_new_swapchain(std::vector<vk::ImageView> const& views, vk::Extent2D size, vk::SurfaceFormatKHR format) noexcept = 0;

    /** Draw using vulkan API.
     *
     * @param swapchain_index The index of the image-view of the swap-chain to draw into.
     * @param start The semaphore used to signal when the swapchain-image is ready to be drawn.
     * @param finish The semaphore used to signal when the finishing drawing into the swapchain-image.
     * @param render_area The area of the window that is being drawn.
     */
    virtual void draw(uint32_t swapchain_index, vk::Semaphore start, vk::Semaphore finish, vk::Rect2D render_area) noexcept = 0;
};

} // namespace hi::inline v1
