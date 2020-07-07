// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineSDF_TextureMap.hpp"
#include "TTauri/GUI/GUIDevice.hpp"

namespace tt::PipelineSDF {

void TextureMap::transitionLayout(const GUIDevice &device, vk::Format format, vk::ImageLayout nextLayout) {
    if (layout != nextLayout) {
        device.transitionLayout(image, format, layout, nextLayout);
        layout = nextLayout;
    }
}

}
