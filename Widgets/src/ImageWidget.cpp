// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ImageWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include <cmath>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;


ImageWidget::ImageWidget(Window &window, Widget *parent, URL path) noexcept :
    Widget(window, parent), path(std::move(path))
{
}

void ImageWidget::drawBackingImage() noexcept
{
    if (backingImage->state == GUI::PipelineImage::Image::State::Uploaded) {
        return;
    }
    backingImage->state = GUI::PipelineImage::Image::State::Drawing;

    auto vulkanDevice = device();

    auto linearMap = PixelMap<R16G16B16A16SFloat>{ backingImage->extent };
    fill(linearMap, vec::color(0.0, 0.0, 0.0));

    // Draw image in the fullPixelMap.
    // XXX This probably should allocate a PixelMap and add it to this class.
    loadPNG(linearMap, path);

    let text_style = TextStyle("Arial", FontVariant{}, 8, vec{ 0.5f, 1.0f, 0.5f, 1.0f }, 0.0, TextDecoration{});
    let shaped_text = ShapedText("g", text_style);
    let glyph = shaped_text.get_path();

    // Draw something.
    let color = vec::color(0.5f, 1.0f, 0.5f);
    let path1 = mat::T(20.0, 30.0) * glyph;
    composit(linearMap, color, path1);

    let path2 = mat::T(30.0, 30.0) * glyph;
    composit(linearMap, color, path2);

    let path3 = mat::T(40.0, 30.0) * glyph;
    composit(linearMap, color, path3);

    vulkanDevice->imagePipeline->uploadPixmapToAtlas(*backingImage, linearMap);
}

void ImageWidget::updateAndPlaceVertices(
    cpu_utc_clock::time_point displayTimePoint,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    clearAndPickleAppend(key, "ImageView", box.currentExtent(), path);

    auto vulkanDevice = device();

    // backingImage keeps track of use count.
    backingImage = vulkanDevice->imagePipeline->getImage(key, box.currentExtent());
    drawBackingImage();

    let origin = vec{backingImage->extent} * vec{-0.5};

    GUI::PipelineImage::ImageLocation location;
    let O = mat::T(origin);
    let R = mat::R(rotation);
    let T = mat::T(box.currentOffset(elevation));
    location.transform = T * R * O;
    location.clippingRectangle = box.currentRectangle();

    backingImage->placeVertices(location, image_vertices);

    Widget::updateAndPlaceVertices(displayTimePoint, flat_vertices, box_vertices, image_vertices, sdf_vertices);
}

}
