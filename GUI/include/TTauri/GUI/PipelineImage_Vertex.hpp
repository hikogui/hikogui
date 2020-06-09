// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/PipelineImage_ImageLocation.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/aarect.hpp"
#include "TTauri/Foundation/R32G32B32SFloat.hpp"
#include "TTauri/Foundation/R32G32B32A32SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri::PipelineImage {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct Vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    R32G32B32SFloat position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    R32G32B32A32SFloat clippingRectangle;

    //! The x, y coordinate inside the texture-atlas, z is used as an index in the texture-atlas array
    R32G32B32SFloat atlasPosition;

    Vertex(vec position, vec atlasPosition, aarect clippingRectangle) noexcept :
        position(position),
        clippingRectangle(clippingRectangle),
        atlasPosition(atlasPosition) {}

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
            { 2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, atlasPosition) },                
        };
    }
};
}
