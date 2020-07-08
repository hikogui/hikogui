// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GUIDevice_forward.hpp"
#include "../PixelMap.hpp"
#include "../SDF8.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace tt::PipelineSDF {

struct TextureMap {
    vk::Image image;
    VmaAllocation allocation = {};
    vk::ImageView view;
    tt::PixelMap<SDF8> pixelMap;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    void transitionLayout(const GUIDevice &device, vk::Format format, vk::ImageLayout nextLayout);
};

}
