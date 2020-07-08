// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../Path.hpp"
#include "../text/FontGlyphIDs.hpp"
#include "../text/ElusiveIcons.hpp"
#include "../text/TTauriIcons.hpp"
#include <memory>
#include <string>
#include <array>
#include <variant>

namespace tt {

class ToolbarButtonWidget : public Widget {
public:
    bool pressed = false;

    /** This is a close button, show background in red.
     */
    bool closeButton = false;

    using icon_type = std::variant<FontGlyphIDs>;
    icon_type icon;
    std::function<void()> delegate;

    ToolbarButtonWidget(Window &window, Widget *parent, icon_type icon, std::function<void()> delegate) noexcept;

    ToolbarButtonWidget(Window &window, Widget *parent, ElusiveIcon icon, std::function<void()> delegate) noexcept :
        ToolbarButtonWidget(window, parent, to_FontGlyphIDs(icon), std::move(delegate)) {}
    ToolbarButtonWidget(Window &window, Widget *parent, TTauriIcon icon, std::function<void()> delegate) noexcept :
        ToolbarButtonWidget(window, parent, to_FontGlyphIDs(icon), std::move(delegate)) {}

    ~ToolbarButtonWidget() {}

    ToolbarButtonWidget(const ToolbarButtonWidget &) = delete;
    ToolbarButtonWidget &operator=(const ToolbarButtonWidget &) = delete;
    ToolbarButtonWidget(ToolbarButtonWidget &&) = delete;
    ToolbarButtonWidget &operator=(ToolbarButtonWidget &&) = delete;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(MouseEvent const &event) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

private:
    int state() const noexcept;
};

}
