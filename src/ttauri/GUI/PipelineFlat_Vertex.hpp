// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../vspan.hpp"
#include "../vec.hpp"
#include "../aarect.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include "../R32G32B32A32SFloat.hpp"
#include "../R32G32B32SFloat.hpp"
#include <vulkan/vulkan.hpp>
#include <nonstd/span>

namespace tt::PipelineFlat {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct Vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    R32G32B32SFloat position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    R32G32B32A32SFloat clippingRectangle;

    //! transparency of the image.
    R16G16B16A16SFloat color;


    Vertex(vec position, aarect clippingRectangle, vec color) noexcept :
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
