// Copyright 2019 Pokitec
// All rights reserved.

#include "ImageWidget.hpp"
#include "utils.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>

namespace TTauri::GUI::Widgets {

using namespace std::literals;


ImageWidget::ImageWidget(const std::filesystem::path path) :
    Widget(), path(std::move(path))
{
}

void ImageWidget::drawBackingImage()
{
    if (backingImage->state == GUI::PipelineImage::Image::State::Uploaded) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBA>{ backingImage->extent };
    fill(linearMap, wsRGBA{ 0x000000ff });

    // Draw image in the fullPixelMap.
    // XXX This probably should allocate a PixelMap and add it to this class.
    Draw::loadPNG(linearMap, path);

    let myFont = getResource<Draw::Font>(URL("resource:Themes/Fonts/Roboto/Roboto-Regular.ttf"));
    let glyphIndex = myFont.characterMap.at('g');
    let glyph = myFont.glyphs.at(glyphIndex);

    // Draw something.
    let color = wsRGBA{ 0.5f, 1.0f, 0.5f, 1.0f };
    let path1 = T2D({20.0, 30.0}, 8.0) * glyph;
    composit(linearMap, color, path1, Draw::SubpixelOrientation::Unknown);

    let path2 = T2D({30.0, 30.0}, 8.0) * glyph;
    composit(linearMap, color, path2, Draw::SubpixelOrientation::RedLeft);

    let path3 = T2D({40.0, 30.0}, 8.0) * glyph;
    composit(linearMap, color, path3, Draw::SubpixelOrientation::RedRight);

    auto fullPixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(backingImage->extent);
    fill(fullPixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(*backingImage);
    backingImage->state = GUI::PipelineImage::Image::State::Uploaded;
}

void ImageWidget::pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex> &vertices, size_t &offset)
{
    std::string key;
    key ^ "ImageView" ^ box.currentExtent() ^ path.u8string();

    auto vulkanDevice = device();

    // backingImage keeps track of use count.
    backingImage = vulkanDevice->imagePipeline->getImage(key, box.currentExtent());
    drawBackingImage();

    //rotation = fmod(rotation + 0.001, boost::math::constants::pi<double>() * 2.0);

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
