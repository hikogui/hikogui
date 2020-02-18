// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include <vulkan/vulkan.hpp>
#include <gsl/gsl>

namespace TTauri::GUI::PipelineFlat {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct Vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    glm::vec2 position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    rect2 clippingRectangle;

    //! transparency of the image.
    glm::vec4 color;

    //! The depth for depth test.
    uint16_t depth;

    //! Align to 32 bits.
    uint8_t dummy1;
    uint8_t dummy2;

    Vertex(glm::vec2 position, glm::vec4 color, rect2 clippingRectangle) noexcept :
        position(position),
        clippingRectangle(clippingRectangle),
        color(color),
        depth(static_cast<uint16_t>(0)),
        dummy1(0), dummy2(0) {}

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
            { 3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color) },
            { 4, 0, vk::Format::eR16Uint, offsetof(Vertex, depth) },
        };
    }

    static void placeBox(vspan<Vertex> &vertices, rect2 box, glm::vec4 color, rect2 clippingRectangle)
    {
        vertices.emplace_back(glm::vec2{box.offset.x, box.offset.y}, color, clippingRectangle);
        vertices.emplace_back(glm::vec2{box.offset.x + box.extent.width(), box.offset.y}, color, clippingRectangle);
        vertices.emplace_back(glm::vec2{box.offset.x + box.extent.width(), box.offset.y + box.extent.height()}, color, clippingRectangle);
        vertices.emplace_back(glm::vec2{box.offset.x, box.offset.y + box.extent.height()}, color, clippingRectangle);
    }
};
}
