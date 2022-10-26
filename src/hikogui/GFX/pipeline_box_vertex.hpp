// Copyright Take Vos 2020-2021.
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

namespace hi::inline v1::pipeline_box {

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

    /** Double 2D coordinates inside the quad, used to determine the distance from the sides and corner inside the fragment
     * shader. x = Number of pixels to the right from the left edge of the quad. y = Number of pixels above the bottom edge. z =
     * Number of pixels to the left from the right edge of the quad. w = Number of pixels below the top edge.
     *
     * The rasteriser will interpolate these numbers, so that inside the fragment shader
     * the distance from a corner can be determined easily.
     */
    sfloat_rgba32 corner_coordinate;

    /** Shape of each corner, negative values are cut corners, positive values are rounded corners.
     */
    sfloat_rgba32 corner_radii;

    /** background color of the box.
     */
    sfloat_rgba16 fill_color;

    /** border color of the box.
     */
    sfloat_rgba16 line_color;


    float line_width;

    vertex(
        sfloat_rgba32 position,
        sfloat_rgba32 clipping_rectangle,
        sfloat_rgba32 corner_coordinate,
        sfloat_rgba32 corner_radii,
        sfloat_rgba16 fill_color,
        sfloat_rgba16 line_color,
        float line_width) noexcept :
        position(position),
        clipping_rectangle(clipping_rectangle),
        corner_coordinate(corner_coordinate),
        corner_radii(corner_radii),
        fill_color(fill_color),
        line_color(line_color),
        line_width(line_width)
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
            {2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, corner_coordinate)},
            {3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vertex, corner_radii)},
            {4, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(vertex, fill_color)},
            {5, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(vertex, line_color)},
            {6, 0, vk::Format::eR32Sfloat, offsetof(vertex, line_width)},
        };
    }
};

} // namespace hi::inline v1::pipeline_box
