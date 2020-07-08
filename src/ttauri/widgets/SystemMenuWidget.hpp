// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../GUI/PipelineImage_Image.hpp"
#include "../Path.hpp"
#include "../cells/Image.hpp"
#include "../cells/ImageCell.hpp"
#include <memory>
#include <string>
#include <array>


namespace tt {

class SystemMenuWidget : public Widget {
    std::unique_ptr<ImageCell> iconCell;

    aarect systemMenuRectangle;

public:
    SystemMenuWidget(Window &window, Widget *parent, Image const &icon) noexcept;
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
