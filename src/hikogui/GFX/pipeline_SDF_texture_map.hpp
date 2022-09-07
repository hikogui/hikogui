// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../pixel_map.hpp"
#include "../rapid/sdf_r8.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace hi::inline v1 {
class gfx_device_vulkan;

namespace pipeline_SDF {

struct texture_map {
    vk::Image image;
    VmaAllocation allocation = {};
    vk::ImageView view;
    hi::pixel_map<sdf_r8> pixel_map;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    void transitionLayout(const gfx_device_vulkan &device, vk::Format format, vk::ImageLayout nextLayout);
};

} // namespace pipeline_SDF
} // namespace hi::inline v1
