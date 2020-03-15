// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"

namespace TTauri::GUI::PipelineImage {

/*! Information on the location and orientation of an image on a window.
*/
struct ImageLocation {
    //! The pixel-coordinates where the origin is located relative to the top-left corner of the window.
    vec position = {};

    //! Location of the origin relative to the top-left of the image in number of pixels.
    vec origin = {};

    //! Scale of the image around the origin before rotation.
    vec scale = 1.0;

    //! Clockwise rotation around the origin of the image in radials.
    float rotation = 0.0;

    //! The position in pixels of the clipping rectangle relative to the top-left corner of the window, and extent in pixels.
    rect clippingRectangle;

    bool operator==(const ImageLocation &other) noexcept {
        return (
            position == other.position &&
            origin == other.origin &&
            rotation == other.rotation &&
            clippingRectangle == other.clippingRectangle
        );
    }

    bool operator!=(const ImageLocation &other) noexcept { return !((*this) == other); }
};

}
