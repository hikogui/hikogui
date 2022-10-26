// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/axis_aligned_rectangle.hpp"
#include "../rapid/sfloat_rgba32.hpp"
#include <vulkan/vulkan.hpp>

namespace hi::inline v1::pipeline_image {

/*! A vertex defining a rectangle on a window.
 * The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
 */
struct alignas(16) vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    sfloat_rgba32 position;

    //! The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
    sfloat_rgba32 clipping_rectangle;

    //! The x, y coordinate inside the texture-atlas, z is used as an index in the texture-atlas array
    sfloat_rgba32 atlas_position;

    vertex(sfloat_rgba32 position, sfloat_rgba32 clipping_rectangle, sfloat_rgba32 atlas_position) noexcept :
        position(position), clipping_rectangle(clipping_rectangle), atlas_position(atlas_position)
    {
    }

    static vk::VertexInputBindingDescription inputBindingDescription()
    {
        return {0, sizeof(vertex), vk::VertexInputRate::eVertex};
    }

    static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions()
    {
        return {
            {0, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, position)},
            {1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, clipping_rectangle)},
            {2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, atlas_position)},
        };
    }
};

} // namespace hi::inline v1::pipeline_image
