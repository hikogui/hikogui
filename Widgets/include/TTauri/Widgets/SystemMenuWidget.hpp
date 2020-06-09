// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/GUI/PipelineImage_Image.hpp"
#include "TTauri/Foundation/Path.hpp"
#include <memory>
#include <string>
#include <array>


namespace TTauri {

class SystemMenuWidget : public Widget {
    PixelMap<R16G16B16A16SFloat> image;
    PipelineImage::Image backingImage;

    aarect systemMenuRectangle;

public:
    SystemMenuWidget(Window &window, Widget *parent, PixelMap<R16G16B16A16SFloat> &&image) noexcept;
    ~SystemMenuWidget() {}

    SystemMenuWidget(const SystemMenuWidget &) = delete;
    SystemMenuWidget &operator=(const SystemMenuWidget &) = delete;
    SystemMenuWidget(SystemMenuWidget &&) = delete;
    SystemMenuWidget &operator=(SystemMenuWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override;
    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

private:

};

}
