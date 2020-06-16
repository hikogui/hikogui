// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/Foundation/Path.hpp"
#include <memory>
#include <string>
#include <array>

namespace tt {
struct Path;
}

namespace tt {

class ImageWidget : public Widget {
public:

    PixelMap<R16G16B16A16SFloat> image;

    ImageWidget(Window &window, Widget *parent, PixelMap<R16G16B16A16SFloat> image) noexcept;
    ~ImageWidget() {}

    ImageWidget(const ImageWidget &) = delete;
    ImageWidget &operator=(const ImageWidget &) = delete;
    ImageWidget(ImageWidget &&) = delete;
    ImageWidget &operator=(ImageWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override;
    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(MouseEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

private:
    std::tuple<aarect, aarect, aarect, aarect> getButtonRectangles() const noexcept;

    PixelMap<R16G16B16A16SFloat> drawApplicationIconImage(PipelineImage::Image &image) noexcept;
    PixelMap<R16G16B16A16SFloat> drawTrafficLightsImage(PipelineImage::Image &image) noexcept;

    static void drawCross(Path &path, vec position, float radius) noexcept;
    static void drawTrianglesOutward(Path &path, vec position, float radius) noexcept;
    static void drawTrianglesInward(Path &path, vec position, float radius) noexcept;

    PipelineImage::Backing::ImagePixelMap drawImage(std::shared_ptr<PipelineImage::Image> image) noexcept;

    PipelineImage::Backing backingImage; 
};

}
