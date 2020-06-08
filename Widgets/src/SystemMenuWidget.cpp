// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Widgets/SystemMenuWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Text/TTauriIcons.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <Windows.h>
#include <WinUser.h>
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {


SystemMenuWidget::SystemMenuWidget(Window &window, Widget *parent, PixelMap<R16G16B16A16SFloat> &&image) noexcept :
    Widget(window, parent, window.systemMenuButtonExtent),
    image(std::move(image)),
    systemMenuRectangle(window.systemMenuButtonExtent)
{
}


void SystemMenuWidget::layout(hires_utc_clock::time_point displayTimePoint) noexcept
{
    Widget::layout(displayTimePoint);

    backingImage = device()->imagePipeline->makeImage(rectangle().extent());
    backingImage.upload(image);
}

void SystemMenuWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    switch (backingImage.state) {
    case PipelineImage::Image::State::Drawing:
        forceRedraw = true;
        break;

    case PipelineImage::Image::State::Uploaded: {
        auto context = drawContext;
        context.transform = context.transform * mat::S::uniform2D(extent(), backingImage.extent);
        context.drawImage(backingImage);
        } break;

    default:;
    }
}

HitBox SystemMenuWidget::hitBoxTest(vec position) const noexcept
{
    if (systemMenuRectangle.contains(position)) {
        // Only the top-left square should return ApplicationIcon, leave
        // the reset to the toolbar implementation.
        return HitBox{this, elevation, HitBox::Type::ApplicationIcon};
    } else {
        return {};
    }
}

}
