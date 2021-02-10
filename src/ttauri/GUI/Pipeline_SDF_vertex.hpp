// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include "../R32G32B32A32SFloat.hpp"
#include "../R32G32B32SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace tt::pipeline_SDF {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    R32G32B32SFloat position;

    //! Clipping rectangle. (x,y)=bottom-left, (z,w)=top-right
    R32G32B32A32SFloat clippingRectangle;

    //! The x, y (relative to bottom-left) coordinate inside the texture-atlas, z is used as an index in the texture-atlas array
    R32G32B32SFloat textureCoord;

    //! The color of the glyph.
    R16G16B16A16SFloat color;

    vertex(f32x4 position, aarect clippingRectangle, f32x4 textureCoord, f32x4 color) noexcept :
        position(position),
        clippingRectangle(clippingRectangle),
        textureCoord(textureCoord),
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
            { 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, clippingRectangle) },
            { 2, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, textureCoord) },                
            { 3, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(vertex, color) }
        };
    }
};
}
