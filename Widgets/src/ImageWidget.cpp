// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ImageWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include <cmath>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;


ImageWidget::ImageWidget(URL path) noexcept :
    Widget(), path(std::move(path))
{
}

void ImageWidget::drawBackingImage() noexcept
{
    if (backingImage->state == GUI::PipelineImage::Image::State::Uploaded) {
        return;
    }
    backingImage->state = GUI::PipelineImage::Image::State::Drawing;

    auto vulkanDevice = device();

    auto linearMap = PixelMap<wsRGBA>{ backingImage->extent };
    fill(linearMap, wsRGBA{ 0x000000ff });

    // Draw image in the fullPixelMap.
    // XXX This probably should allocate a PixelMap and add it to this class.
    loadPNG(linearMap, path);

    let text_style = TextStyle("Arial", FontVariant{}, 8, wsRGBA{ 0.5f, 1.0f, 0.5f, 1.0f }, TextDecoration{});
    let shaped_text = ShapedText("g", text_style, Alignment::BottomLeft, extent2{10.0, 50.0}, extent2{100.0, 500.0});
    let glyph = shaped_text.get_path();

    // Draw something.
    let color = wsRGBA{ 0.5f, 1.0f, 0.5f, 1.0f };
    let path1 = T2D({20.0, 30.0}) * glyph;
    composit(linearMap, color, path1, SubpixelOrientation::Unknown);

    let path2 = T2D({30.0, 30.0}) * glyph;
    composit(linearMap, color, path2, SubpixelOrientation::RedLeft);

    let path3 = T2D({40.0, 30.0}) * glyph;
    composit(linearMap, color, path3, SubpixelOrientation::RedRight);

    vulkanDevice->imagePipeline->uploadPixmapToAtlas(*backingImage, linearMap);
}

void ImageWidget::pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex> &vertices, ssize_t &offset) noexcept
{
    clearAndPickleAppend(key, "ImageView", box.currentExtent(), path);

    auto vulkanDevice = device();

    // backingImage keeps track of use count.
    backingImage = vulkanDevice->imagePipeline->getImage(key, box.currentExtent());
    drawBackingImage();

    GUI::PipelineImage::ImageLocation location;
    location.depth = depth + 0.0f;
    location.origin = {backingImage->extent.x * 0.5, backingImage->extent.y * 0.5};
    location.position = box.currentPosition() + location.origin;
    location.rotation = rotation;
    location.alpha = 1.0;
    location.clippingRectangle = box.currentRectangle();

    backingImage->placeVertices(location, vertices, offset);
}

}
