// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include <vulkan/vulkan.hpp>
#include <gsl/gsl>

namespace TTauri::GUI::PipelineBox {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct Vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    glm::vec3 position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    glm::vec4 clippingRectangle;

    //! Double 2D coordinates inside the quad, used to determine the distance from the sides and corner inside the fragment shader.
    glm::vec4 cornerCoordinate;

    //! background color of the box.
    R16G16B16A16SFloat backgroundColor;

    //! border color of the box.
    R16G16B16A16SFloat borderColor;

    //! Shape of each corner, negative values are cut corners, positive values are rounded corners.
    R16G16B16A16SFloat cornerShapes;

    float borderSize;
    float shadowSize;

    Vertex(
        glm::vec3 position,
        glm::vec4 cornerCoordinate,
        R16G16B16A16SFloat backgroundColor,
        float borderSize,
        R16G16B16A16SFloat borderColor,
        float shadowSize,
        R16G16B16A16SFloat cornerShapes,
        rect2 clippingRectangle
    ) noexcept :
        position(position),
        cornerCoordinate(cornerCoordinate),
        clippingRectangle(
            clippingRectangle.offset.x, clippingRectangle.offset.y,
            clippingRectangle.offset.x + clippingRectangle.extent.width(), clippingRectangle.offset.y + clippingRectangle.extent.height()
        ),
        backgroundColor(backgroundColor),
        borderColor(borderColor),
        cornerShapes(cornerShapes),
        borderSize(borderSize),
        shadowSize(shadowSize) {}

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
            { 7, 0, vk::Format::eR32Sfloat, offsetof(Vertex, shadowSize) },
        };
    }
};
}
