// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI::PipelineMSDF {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct Vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    glm::vec2 position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    rect2 clippingRectangle;

    //! The x, y (relative to bottom-left) coordinate inside the texture-atlas, z is used as an index in the texture-atlas array
    glm::u16vec3 atlasPosition;

    uint16_t depth;

    //! The depth for depth test.
    R16G16B16A16SFloat color;

    Vertex(glm::vec2 position, rect2 clippingRectangle, glm::u16vec3 atlasPosition, float depth, wsRGBA color) noexcept :
        position(position),
        clippingRectangle(clippingRectangle),
        atlasPosition(atlasPosition),
        depth(static_cast<uint16_t>(depth)),
        color(R16G16B16A16SFloat{color}) {}

    static vk::VertexInputBindingDescription inputBindingDescription()
    {
        return {
            0, sizeof(Vertex), vk::VertexInputRate::eVertex
        };
    }

    static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions()
    {
        return {
            { 0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position) },
            { 1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, clippingRectangle.offset) },
            { 2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, clippingRectangle.extent) },
            { 3, 0, vk::Format::eR16G16B16Uint, offsetof(Vertex, atlasPosition) },                
            { 4, 0, vk::Format::eR16Uint, offsetof(Vertex, depth) },
            { 5, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(Vertex, color) },
        };
    }
};
}
