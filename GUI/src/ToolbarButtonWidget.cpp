// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/ToolbarButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace std::literals;

ToolbarButtonWidget::ToolbarButtonWidget(Window &window, Widget *parent, Path icon, std::function<void()> delegate) noexcept :
    Widget(window, parent), delegate(delegate)
{
    window.addConstraint(box.height == box.width);

    icon.tryRemoveLayers();
    this->icon = std::move(icon);
}

int ToolbarButtonWidget::state() const noexcept {
    int r = 0;
    r |= window.active ? 1 : 0;
    r |= hover ? 2 : 0;
    r |= pressed ? 4 : 0;
    r |= enabled ? 8 : 0;
    return r;
}

PipelineImage::Backing::ImagePixelMap ToolbarButtonWidget::drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept
{

    auto iconImage = PixelMap<R16G16B16A16SFloat>{image->extent};
    if (std::holds_alternative<Path>(icon)) {
        auto p = std::get<Path>(icon).centerScale(vec{image->extent}, 10.0);
        p.closeLayer(vec::color(1.0, 1.0, 1.0));

        fill(iconImage);
        composit(iconImage, p);
    } else {
        no_default;
    }

    if (!(hover || window.active)) {
        desaturate(iconImage, 0.5f);
    }

    auto linearMap = PixelMap<R16G16B16A16SFloat>{image->extent};
    fill(linearMap);

    composit(linearMap, iconImage);
    return { std::move(image), std::move(linearMap) };
}

void ToolbarButtonWidget::updateAndPlaceVertices(
    cpu_utc_clock::time_point displayTimePoint,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    if (pressed) {
        PipelineFlat::DeviceShared::placeVerticesBox(flat_vertices, box.currentRectangle(), pressedBackgroundColor, box.currentRectangle(), elevation);
    } else if (hover && enabled) {
        PipelineFlat::DeviceShared::placeVerticesBox(flat_vertices, box.currentRectangle(), hoverBackgroundColor, box.currentRectangle(), elevation);
    }

    auto drawingBackingImage = backingImage.loadOrDraw(
        window,
        box.currentExtent(),
        [&](auto image) {
            return drawImage(image);
        },
        "ToolbarButtonWidget",
        this,
        state()
    );
    if (drawingBackingImage) {
        ++renderTrigger;
    }

    if (backingImage.image) {
        let currentScale = (box.currentExtent() / vec{backingImage.image->extent}).xy10();

        GUI::PipelineImage::ImageLocation location;
        let T = mat::T(box.currentOffset(elevation));
        let S = mat::S(currentScale);
        location.transform = T * S;
        location.clippingRectangle = box.currentRectangle();

        backingImage.image->placeVertices(location, image_vertices);

        if (backingImage.image->state != PipelineImage::Image::State::Uploaded) {
            ++renderTrigger;
        }
    } else {
        ++renderTrigger;
    }

    Widget::updateAndPlaceVertices(displayTimePoint, flat_vertices, box_vertices, image_vertices, sdf_vertices);
}

void ToolbarButtonWidget::handleMouseEvent(MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (enabled) {
        if (assign_and_compare(pressed, static_cast<bool>(event.down.leftButton))) {
            ++renderTrigger;
        }

        if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            delegate();
        }
    }
}

HitBox ToolbarButtonWidget::hitBoxTest(vec position) noexcept
{
    if (box.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
