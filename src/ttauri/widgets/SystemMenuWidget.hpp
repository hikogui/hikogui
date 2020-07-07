// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/widgets/Widget.hpp"
#include "ttauri/GUI/PipelineImage_Image.hpp"
#include "ttauri/Path.hpp"
#include "ttauri/cells/Image.hpp"
#include "ttauri/cells/ImageCell.hpp"
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
