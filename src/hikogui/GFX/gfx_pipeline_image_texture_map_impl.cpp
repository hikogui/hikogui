// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_pipeline_image_texture_map.hpp"
#include "gfx_device_vulkan_impl.hpp"
#include "../macros.hpp"

namespace hi::inline v1::gfx_pipeline_image {

void texture_map::transitionLayout(const gfx_device &device, vk::Format format, vk::ImageLayout nextLayout)
{
    if (layout != nextLayout) {
        device.transition_layout(image, format, layout, nextLayout);
        layout = nextLayout;
    }
}

} // namespace hi::inline v1::gfx_pipeline_image
