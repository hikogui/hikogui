// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "PipelineSDF_TextureMap.hpp"
#include "gui_device_vulkan.hpp"

namespace tt::PipelineSDF {

void TextureMap::transitionLayout(const gui_device_vulkan &device, vk::Format format, vk::ImageLayout nextLayout)
{
    if (layout != nextLayout) {
        device.transitionLayout(image, format, layout, nextLayout);
        layout = nextLayout;
    }
}

}
