// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/axis_aligned_rectangle.hpp"

namespace tt::pipeline_image {

/*! Information on the location and orientation of an image on a window.
*/
struct ImageLocation {
    /** Transformation matrix.
     */
    matrix3 transform;

    //! The position in pixels of the clipping rectangle relative to the top-left corner of the window, and extent in pixels.
    aarectangle clippingRectangle;

    bool operator==(const ImageLocation &other) noexcept {
        return (
            transform == other.transform &&
            clippingRectangle == other.clippingRectangle
        );
    }

    bool operator!=(const ImageLocation &other) noexcept { return !((*this) == other); }
};

}
