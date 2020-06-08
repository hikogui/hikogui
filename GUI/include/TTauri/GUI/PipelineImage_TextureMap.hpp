// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI::PipelineImage {

struct TextureMap {
    vk::Image image;
    VmaAllocation allocation = {};
    vk::ImageView view;
    TTauri::PixelMap<R16G16B16A16SFloat> pixelMap;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    void transitionLayout(const Device &device, vk::Format format, vk::ImageLayout nextLayout);
};

}
