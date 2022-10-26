// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/axis_aligned_rectangle.hpp"
#include "../rapid/sfloat_rgba16.hpp"
#include "../rapid/sfloat_rgba32.hpp"
#include "../rapid/sfloat_rgb32.hpp"
#include <vulkan/vulkan.hpp>

namespace hi::inline v1::pipeline_SDF {

/*! A vertex defining a rectangle on a window.
 * The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
 */
struct vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    sfloat_rgb32 position;

    //! Clipping rectangle. (x,y)=bottom-left, (z,w)=top-right
    sfloat_rgba32 clippingRectangle;

    //! The x, y (relative to bottom-left) coordinate inside the texture-atlas, z is used as an index in the texture-atlas array
    sfloat_rgb32 textureCoord;

    //! The color of the glyph.
    sfloat_rgba16 color;

    vertex(point3 position, aarectangle clippingRectangle, point3 textureCoord, hi::color color) noexcept :
        position(position), clippingRectangle(clippingRectangle), textureCoord(textureCoord), color(color)
    {
    }

    static vk::VertexInputBindingDescription inputBindingDescription()
    {
        return {0, sizeof(vertex), vk::VertexInputRate::eVertex};
    }

    static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions()
    {
        return {
            {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, position)},
            {1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, clippingRectangle)},
            {2, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, textureCoord)},
            {3, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(vertex, color)}};
    }
};

} // namespace hi::inline v1::pipeline_SDF
