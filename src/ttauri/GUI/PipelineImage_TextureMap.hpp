// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../PixelMap.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace tt {
class gui_device_vulkan;
}

namespace tt::PipelineImage {

struct TextureMap {
    vk::Image image;
    VmaAllocation allocation = {};
    vk::ImageView view;
    tt::PixelMap<R16G16B16A16SFloat> pixelMap;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    void transitionLayout(const gui_device_vulkan &device, vk::Format format, vk::ImageLayout nextLayout);
};

}
