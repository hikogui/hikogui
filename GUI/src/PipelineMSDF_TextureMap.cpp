// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineMSDF_TextureMap.hpp"
#include "TTauri/GUI/Device.hpp"

namespace TTauri::GUI::PipelineMSDF {

void TextureMap::transitionLayout(const Device &device, vk::Format format, vk::ImageLayout nextLayout) {
    if (layout != nextLayout) {
        device.transitionLayout(image, format, layout, nextLayout);
        layout = nextLayout;
    }
}

}
