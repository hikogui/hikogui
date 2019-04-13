
#include "PipelineImage_TextureMap.hpp"
#include "Device_vulkan.hpp"

namespace TTauri::GUI::PipelineImage {

void TextureMap::transitionLayout(const Device_vulkan &device, vk::Format format, vk::ImageLayout nextLayout) {
    if (layout != nextLayout) {
        device.transitionLayout(image, format, layout, nextLayout);
        layout = nextLayout;
    }
}

}
