// Copyright 2019 Pokitec
// All rights reserved.

#include "ImageWidget.hpp"
#include "utils.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>

namespace TTauri::Widgets {

using namespace std::literals;


ImageWidget::ImageWidget(const std::filesystem::path path) :
    Widget(), path(std::move(path))
{
}

void ImageWidget::drawBackingImage()
{
    if (backingImage->drawn) {
        return;
    }

    auto vulkanDevice = device();

    auto linearMap = Draw::PixelMap<wsRGBApm>{ backingImage->extent };
    linearMap.fill(0x0000'0000'0000'ffffULL);

    // Draw image in the fullPixelMap.
    // XXX This probably should allocate a PixelMap and add it to this class.
    Draw::loadPNG(linearMap, path);

    let myFont = Draw::fonts->get("Themes/Fonts/Roboto/Roboto-Regular.ttf");
    let glyphIndex = myFont.characterMap.at('g');
    let glyph = myFont.glyphs.at(glyphIndex);

    // Draw something.
    let color = wsRGBApm{ 0.5f, 1.0f, 0.5f, 1.0f };
    auto path1 = Draw::Path();
    path1.addGlyph(glyph, {20.0, 30.0}, 8.0);
    path1.fill(linearMap, color , Draw::SubpixelMask::Orientation::Unknown);

    auto path2 = Draw::Path();
    path2.addGlyph(glyph, { 30.0, 30.0 }, 8.0);
    path2.fill(linearMap, color, Draw::SubpixelMask::Orientation::RedLeft);

    auto path3 = Draw::Path();
    path3.addGlyph(glyph, { 40.0, 30.0 }, 8.0);
    path3.fill(linearMap, color, Draw::SubpixelMask::Orientation::RedRight);

    auto fullPixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(backingImage->extent);
    copyLinearToGamma(fullPixelMap, linearMap);
    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(*backingImage);
    backingImage->drawn = true;
}

void ImageWidget::pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex> &vertices, size_t &offset)
{
    let key = BinaryKey("ImageView"s, box.currentExtent(), path.u8string());

    auto vulkanDevice = device();

    // backingImage keeps track of use count.
    vulkanDevice->imagePipeline->exchangeImage(backingImage, key, box.currentExtent());
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
