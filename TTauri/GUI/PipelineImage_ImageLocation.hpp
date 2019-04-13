
#pragma once

#include "TTauri/geometry.hpp"

namespace TTauri::GUI::PipelineImage {

/*! Information on the location and orientation of an image on a window.
*/
struct ImageLocation {
    //! The pixel-coordinates where the origin is located relative to the top-left corner of the window.
    glm::vec2 position;

    //! Location of the origin relative to the top-left of the image in number of pixels.
    glm::vec2 origin;

    //! Clockwise rotation around the origin of the image in radials.
    float rotation;

    //! The position in pixels of the clipping rectangle relative to the top-left corner of the window, and extent in pixels.
    u16rect2 clippingRectangle;

    //! Depth location of the rendered image.
    uint16_t depth;

    //! Transparency of the image.
    uint8_t alpha;
};

}