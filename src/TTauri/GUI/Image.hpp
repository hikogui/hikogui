// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/GUI/Window_forward.hpp"
#include "TTauri/GUI/PipelineImage_Image.hpp"
#include <variant>

namespace tt {
class DrawContext;

/** An image, in different formats.
 */
class Image {
    using image_type = std::variant<FontGlyphIDs,PixelMap<R16G16B16A16SFloat>>;

    image_type image;

    aarect boundingBox;
    PipelineImage::Image backing;

public:
    Image(URL const &url);
    Image(PixelMap<R16G16B16A16SFloat> &&image) noexcept;
    Image(FontGlyphIDs const &glyph) noexcept;

    Image() = default;
    Image(Image const &) noexcept;
    Image(Image &&) noexcept = default;
    Image &operator=(Image const &) noexcept;
    Image &operator=(Image &&) noexcept = default;

    void prepareForDrawing(Window &device) noexcept;

    /** Draw the image.
     *
     * @param drawContext The current draw context.
     * @param rectangle The position and size of the image.
     * @return true when a redraw is needed.
     */
    [[nodiscard]] bool draw(DrawContext const &drawContext, aarect rectangle) noexcept;
};


}
