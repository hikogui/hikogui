// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_image_image_location.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../color/sfloat_rgb32.hpp"
#include "../color/sfloat_rgba32.hpp"
#include <vulkan/vulkan.hpp>

namespace tt::pipeline_image {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    sfloat_rgb32 position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    sfloat_rgba32 clippingRectangle;

    //! The x, y coordinate inside the texture-atlas, z is used as an index in the texture-atlas array
    sfloat_rgb32 atlasPosition;

    vertex(point3 position, point3 atlasPosition, aarectangle clippingRectangle) noexcept :
        position(position),
        clippingRectangle(clippingRectangle),
        atlasPosition(atlasPosition) {}

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
            { 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, clippingRectangle) },
            { 2, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, atlasPosition) },                
        };
    }
};
}
