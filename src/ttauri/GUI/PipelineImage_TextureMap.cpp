// Copyright 2019 Pokitec
// All rights reserved.

#include "PipelineImage_TextureMap.hpp"
#include "gui_device_vulkan.hpp"

namespace tt::PipelineImage {

void TextureMap::transitionLayout(const gui_device_vulkan &device, vk::Format format, vk::ImageLayout nextLayout) {
    if (layout != nextLayout) {
        device.transitionLayout(image, format, layout, nextLayout);
        layout = nextLayout;
    }
}

}
