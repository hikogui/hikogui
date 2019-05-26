// Copyright 2019 Pokitec
// All rights reserved.

#include "ImageView.hpp"
#include "PipelineImage_Image.hpp"
#include "Device_vulkan.hpp"
#include "PipelineImage_DeviceShared.hpp"
#include "PipelineImage_Image.hpp"
#include "PipelineImage_ImageLocation.hpp"
#include "TTauri/Draw/PNG.hpp"
#include "TTauri/Draw/Path.hpp"
#include "TTauri/Draw/Fonts.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>

namespace TTauri::GUI {

ImageView::ImageView(const std::filesystem::path path) :
    View(), path(std::move(path))
{
}

void ImageView::drawBackingImage()
{
    if (backingImage->drawn) {
        return;
    }

    auto vulkanDevice = device<Device_vulkan>();

    auto fullPixelMap = vulkanDevice->imagePipeline->getStagingPixelMap(backingImage->extent);
    fullPixelMap.fill({{0.0f, 0.0f, 0.0f, 1.0f}});

    // Draw image in the fullPixelMap.
    Draw::loadPNG(fullPixelMap, path);

    let myFont = get_singleton<Draw::Fonts>()->get("Themes/Fonts/Roboto/Roboto-Regular.ttf");
    let glyphIndex = myFont.characterMap.at('g');
    let glyph = myFont.glyphs.at(glyphIndex);

    // Draw something.
    let color = color_cast<Color_sRGBLinear>(Color_sRGB{ glm::vec4{0.5f, 1.0f, 0.5f, 1.0f} });
    auto path1 = Draw::Path();
    path1.addGlyph(glyph, {20.0, 30.0}, 8.0);
    path1.render(fullPixelMap, color , Draw::SubpixelMask::Orientation::Unknown);

    auto path2 = Draw::Path();
    path2.addGlyph(glyph, { 30.0, 30.0 }, 8.0);
    path2.render(fullPixelMap, color, Draw::SubpixelMask::Orientation::RedLeft);

    auto path3 = Draw::Path();
    path3.addGlyph(glyph, { 40.0, 30.0 }, 8.0);
    path3.render(fullPixelMap, color, Draw::SubpixelMask::Orientation::RedRight);


    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(*backingImage);
    backingImage->drawn = true;
}

void ImageView::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset)
{
    auto key = (boost::format("ImageView(%i,%i,%s)") % extent.x % extent.y % path).str();

    auto vulkanDevice = device<Device_vulkan>();

    // backingImage keeps track of use count.
    vulkanDevice->imagePipeline->exchangeImage(backingImage, key, extent);
    drawBackingImage();

    //rotation = fmod(rotation + 0.001, boost::math::constants::pi<double>() * 2.0);

    PipelineImage::ImageLocation location;
    location.position = position;
    location.depth = depth + 0.0;
    location.origin = {backingImage->extent.x * 0.5, backingImage->extent.y * 0.5};
    location.position = position + location.origin;
    location.rotation = rotation;
    location.alpha = 1.0;
    location.clippingRectangle = {
        {position.x, position.y},
        extent - u64vec2(10, 10)
    };

    backingImage->placeVertices(location, vertices, offset);
}

}
