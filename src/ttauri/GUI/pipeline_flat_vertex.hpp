// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../vspan.hpp"
#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include "../color/sfloat_rgba16.hpp"
#include "../color/sfloat_rgba32.hpp"
#include "../color/sfloat_rgb32.hpp"
#include <vulkan/vulkan.hpp>
#include <span>

namespace tt::pipeline_flat {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    sfloat_rgb32 position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    sfloat_rgba32 clipping_rectangle;

    //! transparency of the image.
    sfloat_rgba16 color;


    vertex(aarect clippingRectangle, point3 position, tt::color color) noexcept :
        position(position),
        clipping_rectangle(clippingRectangle),
        color(color) {}

    static vk::VertexInputBindingDescription inputBindingDescription()
    {
        return {
            0, sizeof(vertex), vk::VertexInputRate::eVertex
        };
    }

    static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions()
    {
        return {
            { 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, position) },
            { 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, clipping_rectangle) },
            { 3, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(vertex, color) }
        };
    }
};

}
