// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/A8B8G8R8SrgbPack32.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI::PipelineImage {

struct TextureMap {
    vk::Image image;
    VmaAllocation allocation = {};
    vk::ImageView view;
    TTauri::PixelMap<A8B8G8R8SrgbPack32> pixelMap;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    void transitionLayout(const Device &device, vk::Format format, vk::ImageLayout nextLayout);
};

}
