//
//  Rectangle.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Vector.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri {
namespace Toolkit {
namespace GUI {

/*! An description of a rectangle positioned somewhere on a parent rectangle.
 *
 * The coordinate system has:
 * - increasing x toward the right.
 * - increasing y toward the bottom.
 *
 * Origin of a screen is the center of the left-top pixel.
 * Origin of a `Window` is the center of the left-top pixel.
 *
 * The `offset`, `size` and `origin` of a `Window` are in physicial screen pixels.
 *
 * Containers should only zoom out their child widgets. This means a child widget only
 * needs the DPI of the screen to calculate its size.
 */
struct Rectangle {
    //! Offset from the origin of the parent `Rectangle`, to the origin of this `Rectangle`.
    float2 offset;

    //! Size of this `Rectangle`.
    VkExtent2D size;

    //! Origin within this `Rectangle`, in reference to the center of the non-rotated left-top pixel of this `Rectangle`.
    float2 origin;

    //! Rotation in radials, clock-wise, around the origin.
    float rotation;

    //! Scale around the origin.
    float scale;

    //! Level above the parent `Rectangle`.
    float zIndex;

    Rectangle(void) {
        offset = {0.0, 0.0};
        size = {0, 0};
        origin = {0.0, 0.0};
        rotation = 0.0;
        scale = 0.0;
        zIndex = 0.0;
    }
};

}}}
