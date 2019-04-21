//
//  ImageView.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "ImageView.hpp"
#include "PipelineImage_Image.hpp"
#include "Device_vulkan.hpp"
#include "PipelineImage_DeviceShared.hpp"
#include "PipelineImage_Image.hpp"
#include "PipelineImage_ImageLocation.hpp"

#include "TTauri/Draw/PNG.hpp"

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

    auto fullPixelMap = vulkanDevice->imagePipeline->getStagingPixelMap();

    // Draw image in the fullPixelMap.
    TTauri::Draw::loadPNG(fullPixelMap, path);

    // Draw some text on top of the fullPixelMap.


    vulkanDevice->imagePipeline->updateAtlasWithStagingPixelMap(*backingImage);
    backingImage->drawn = true;
}

void ImageView::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset)
{
    auto key = (boost::format("ImageView(%i,%i,%s)") % extent.x % extent.y % path).str();

    auto vulkanDevice = device<Device_vulkan>();

    vulkanDevice->imagePipeline->exchangeImage(backingImage, key, extent);
    drawBackingImage();

    rotation = fmod(rotation + 0.001, boost::math::constants::pi<double>() * 2.0);

    PipelineImage::ImageLocation location;
    location.position = position;
    location.depth = depth + 0.0;
    location.origin = {backingImage->extent.x * 0.5, backingImage->extent.y * 0.5};
    location.position = position + location.origin;
    location.rotation = rotation;
    location.alpha = 1.0;
    location.clippingRectangle = {
        {position.x, position.y},
        extent
    };

    backingImage->placeVertices(location, vertices, offset);
}

}
