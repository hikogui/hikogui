// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vspan.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Foundation/R32G32B32A32SFloat.hpp"
#include "TTauri/Foundation/R32G32B32SFloat.hpp"
#include <vulkan/vulkan.hpp>
#include <gsl/gsl>

namespace TTauri::GUI::PipelineBox {

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
        vec position,
        vec cornerCoordinate,
        vec backgroundColor,
        float borderSize,
        vec borderColor,
        vec cornerShapes,
        rect clippingRectangle
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
