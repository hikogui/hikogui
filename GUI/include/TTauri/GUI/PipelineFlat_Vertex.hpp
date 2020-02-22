// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include <vulkan/vulkan.hpp>
#include <gsl/gsl>

namespace TTauri::GUI::PipelineFlat {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct Vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    glm::vec3 position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    glm::vec4 clippingRectangle;

    //! transparency of the image.
    R16G16B16A16SFloat color;


    Vertex(glm::vec3 position, glm::vec4 clippingRectangle, R16G16B16A16SFloat color) noexcept :
        position(position),
        clippingRectangle(clippingRectangle),
        color(color) {}

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
            { 3, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(Vertex, color) }
        };
    }
};

}
