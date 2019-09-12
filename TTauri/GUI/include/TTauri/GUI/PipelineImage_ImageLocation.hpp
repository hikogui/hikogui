// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/geometry.hpp"

namespace TTauri::GUI::PipelineImage {

/*! Information on the location and orientation of an image on a window.
*/
struct ImageLocation {
    //! The pixel-coordinates where the origin is located relative to the top-left corner of the window.
    glm::vec2 position = {};

    //! Location of the origin relative to the top-left of the image in number of pixels.
    glm::vec2 origin = {};

    //! Scale of the image around the origin before rotation.
    glm::vec2 scale = {};

    //! Clockwise rotation around the origin of the image in radials.
    float rotation = 0.0;

    //! The position in pixels of the clipping rectangle relative to the top-left corner of the window, and extent in pixels.
    rect2 clippingRectangle = {};

    //! Depth location of the rendered image.
    float depth = 0.0;

    //! Transparency of the image.
    float alpha = 0.0;

    bool operator==(const ImageLocation &other) noexcept {
        return (
            position == other.position &&
            origin == other.origin &&
            rotation == other.rotation &&
            clippingRectangle == other.clippingRectangle &&
            depth == other.depth &&
            alpha == other.alpha
        );
    }

    bool operator!=(const ImageLocation &other) noexcept { return !((*this) == other); }
};

}
