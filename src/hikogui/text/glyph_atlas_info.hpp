// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include "../geometry/module.hpp"

namespace hi::inline v1 {

class glyph_atlas_info {
public:
    /** pixel coordinates.
     *
     * - (x, y): pixel coordinate of left-bottom corner of the glyph in the atlas.
     * - z: index in the texture map array.
     */
    point3 position;

    /** Size of the glyph in pixels in the texture map.
     */
    extent2 size;

    /** The scaling factor used for scaling a quad to include the border.
     */
    scale2 border_scale;

    /** The position and size of the glyph in the texture in UV coordinates.
     *
     * The coordinates are relative values between 0.0 - 1.0 between the edges of the texture map.
     */
    aarectangle texture_coordinates;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return size == extent2{};
    }

    [[nodiscard]] constexpr operator bool() const noexcept
    {
        return not empty();
    }

    constexpr glyph_atlas_info() noexcept = default;

    /**
     *
     * @param position Pixel coordinate (x, y), z: texture map number.
     * @param size Number of pixels in width and height
     * @param border_scale The amount to scale a polygon to add a border.
     * @param texture_coordinate_scale The amount to scale texel coordinate to UV (0.0-1.0) coordinates.
     */
    constexpr glyph_atlas_info(point3 position, extent2 size, scale2 border_scale, scale2 texture_coordinate_scale) noexcept :
        position(position),
        size(size),
        border_scale(border_scale),
        texture_coordinates(bounding_rectangle(texture_coordinate_scale * rectangle{position, size}))
    {
        hi_axiom(position == floor(position));
        hi_axiom(size == ceil(size));
    }
};

} // namespace hi::inline v1
