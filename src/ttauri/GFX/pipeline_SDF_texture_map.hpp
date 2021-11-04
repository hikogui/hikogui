// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../pixel_map.hpp"
#include "../rapid/sdf_r8.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace tt {
inline namespace v1 {
class gfx_device_vulkan;
}
}

namespace tt::pipeline_SDF {
inline namespace v1 {

struct texture_map {
    vk::Image image;
    VmaAllocation allocation = {};
    vk::ImageView view;
    tt::pixel_map<sdf_r8> pixel_map;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    void transitionLayout(const gfx_device_vulkan &device, vk::Format format, vk::ImageLayout nextLayout);
};

}
}
