// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../vector_span.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../rapid/sfloat_rgba16.hpp"
#include "../rapid/sfloat_rgba32.hpp"
#include "../rapid/sfloat_rgb32.hpp"
#include "../rapid/int_abgr8_pack.hpp"
#include <vulkan/vulkan.hpp>
#include <span>

namespace hi::inline v1::pipeline_alpha {

/*! A vertex defining a rectangle on a window.
 * The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
 */
struct alignas(16) vertex {
    /** The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
     */
    sfloat_rgba32 position;

    /** The position in pixels of the clipping rectangle relative to the bottom-left corner of the window, and extent in pixels.
     */
    sfloat_rgba32 clipping_rectangle;

    /** The alpha value of the resulting pixels inside the quad.
     */
    float alpha;

    vertex(sfloat_rgba32 position, sfloat_rgba32 clipping_rectangle, float alpha) noexcept :
        position(position), clipping_rectangle(clipping_rectangle), alpha(alpha)
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
            {2, 0, vk::Format::eR32Sfloat, offsetof(vertex, alpha)},
        };
    }
};

} // namespace hi::inline v1::pipeline_alpha
