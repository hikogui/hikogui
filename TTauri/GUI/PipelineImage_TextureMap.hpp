
#pragma once;

#include "TTauri/Draw/PixelMap.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI {
class Device_vulkan;
}

namespace TTauri::GUI::PipelineImage {

struct TextureMap {
    vk::Image image;
    VmaAllocation allocation = {};
    vk::ImageView view;
    TTauri::Draw::PixelMap<uint32_t> pixelMap;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    void transitionLayout(const Device_vulkan &device, vk::Format format, vk::ImageLayout nextLayout);
};

}
