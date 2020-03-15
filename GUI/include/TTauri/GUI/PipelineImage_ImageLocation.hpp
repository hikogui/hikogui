// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/rect.hpp"

namespace TTauri::GUI::PipelineImage {

/*! Information on the location and orientation of an image on a window.
*/
struct ImageLocation {
    /** Transformation matrix.
     */
    mat transform;

    //! The position in pixels of the clipping rectangle relative to the top-left corner of the window, and extent in pixels.
    rect clippingRectangle;

    bool operator==(const ImageLocation &other) noexcept {
        return (
            transform == other.transform &&
            clippingRectangle == other.clippingRectangle
        );
    }

    bool operator!=(const ImageLocation &other) noexcept { return !((*this) == other); }
};

}
