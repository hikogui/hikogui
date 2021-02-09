// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../vspan.hpp"
#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include "../R32G32B32A32SFloat.hpp"
#include "../R32G32B32SFloat.hpp"
#include <vulkan/vulkan.hpp>
#include <span>

namespace tt::PipelineBox {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct Vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    R32G32B32SFloat position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    R32G32B32A32SFloat clippingRectangle;

    //! Double 2D coordinates inside the quad, used to determine the distance from the sides and corner inside the fragment shader.
    R32G32B32A32SFloat cornerCoordinate;

    //! background color of the box.
    R16G16B16A16SFloat backgroundColor;

    //! border color of the box.
    R16G16B16A16SFloat borderColor;

    //! Shape of each corner, negative values are cut corners, positive values are rounded corners.
    R16G16B16A16SFloat cornerShapes;

    float borderSize;

    Vertex(
        f32x4 position,
        f32x4 cornerCoordinate,
        f32x4 backgroundColor,
        float borderSize,
        f32x4 borderColor,
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
            0, sizeof(Vertex), vk::VertexInputRate::eVertex
        };
    }

    static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions()
    {
        return {
            { 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) },
            { 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, clippingRectangle) },
            { 2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, cornerCoordinate) },
            { 3, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(Vertex, backgroundColor) },
            { 4, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(Vertex, borderColor) },
            { 5, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(Vertex, cornerShapes) },
            { 6, 0, vk::Format::eR32Sfloat, offsetof(Vertex, borderSize) },
        };
    }
};
}
