// Copyright Take Vos 2020.
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

namespace tt::pipeline_box {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    sfloat_rgb32 position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    sfloat_rgba32 clippingRectangle;

    //! Double 2D coordinates inside the quad, used to determine the distance from the sides and corner inside the fragment shader.
    sfloat_rgba32 cornerCoordinate;

    //! background color of the box.
    sfloat_rgba16 backgroundColor;

    //! border color of the box.
    sfloat_rgba16 borderColor;

    //! Shape of each corner, negative values are cut corners, positive values are rounded corners.
    sfloat_rgba16 cornerShapes;

    float borderSize;

    vertex(
        f32x4 position,
        f32x4 cornerCoordinate,
        color backgroundColor,
        float borderSize,
        color borderColor,
        f32x4 cornerShapes,
        aarect clippingRectangle
    ) noexcept :
        position(position),
        clippingRectangle(clippingRectangle),
        cornerCoordinate(cornerCoordinate),
        backgroundColor(backgroundColor),
        borderColor(borderColor),
        cornerShapes(cornerShapes),
        borderSize(borderSize) {}

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
            { 2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, cornerCoordinate) },
            { 3, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(vertex, backgroundColor) },
            { 4, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(vertex, borderColor) },
            { 5, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(vertex, cornerShapes) },
            { 6, 0, vk::Format::eR32Sfloat, offsetof(vertex, borderSize) },
        };
    }
};
}
