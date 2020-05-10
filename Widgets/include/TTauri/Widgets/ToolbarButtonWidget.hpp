// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Text/ElusiveIcons.hpp"
#include <memory>
#include <string>
#include <array>
#include <variant>

namespace TTauri::GUI::Widgets {

class ToolbarButtonWidget : public Widget {
public:
    bool pressed = false;

    /** This is a close button, show background in red.
     */
    bool closeButton = false;

    using icon_type = std::variant<Path,Text::FontGlyphIDs>;
    icon_type icon;
    std::function<void()> delegate;

    ToolbarButtonWidget(Window &window, Widget *parent, icon_type icon, std::function<void()> delegate) noexcept;
    ToolbarButtonWidget(Window &window, Widget *parent, Text::ElusiveIcon icon, std::function<void()> delegate) noexcept;
    ~ToolbarButtonWidget() {}

    ToolbarButtonWidget(const ToolbarButtonWidget &) = delete;
    ToolbarButtonWidget &operator=(const ToolbarButtonWidget &) = delete;
    ToolbarButtonWidget(ToolbarButtonWidget &&) = delete;
    ToolbarButtonWidget &operator=(ToolbarButtonWidget &&) = delete;

    void draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(GUI::MouseEvent const &event) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept override;

private:
    int state() const noexcept;

    PipelineImage::Backing::ImagePixelMap drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept;

    PipelineImage::Backing backingImage; 
};

}
