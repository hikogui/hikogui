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

namespace TTauri::GUI {

ImageView::ImageView(const boost::filesystem::path path) :
    View(), path(std::move(path))
{
}

void ImageView::drawBackingImage()
{
    if (backingImage->drawn) {
        return;
    }


    backingImage->drawn = true;
}

void ImageView::pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, size_t &offset)
{
    auto key = (boost::format("ImageView(%i,%i,%s)") % extent.x % extent.y % path).str();

    auto vulkanDevice = device<Device_vulkan>();

    vulkanDevice->imagePipeline->exchangeImage(backingImage, key, extent);
    drawBackingImage();

    PipelineImage::ImageLocation location;
    location.position = position;
    location.depth = depth + 0.0;
    location.origin = {0.0, 0.0};
    location.rotation = 0.0;
    location.alpha = 1.0;
    location.clippingRectangle = {{0, 0}, {std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::max()}};

    backingImage->placeVertices(location, vertices, offset);
}

}
