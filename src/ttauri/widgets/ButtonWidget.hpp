// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include <rhea/constraint.hpp>
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class ButtonWidget : public Widget {
protected:
    bool value = false;
    bool pressed = false;

    std::string label = "<unknown>";

    ShapedText labelShapedText;
    mat::T textTranslate;
public:

    ButtonWidget(Window &window, Widget *parent, std::string const label) noexcept;
    ~ButtonWidget();

    bool layout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(MouseEvent const &event) noexcept override;
    void handleCommand(command command) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
    [[nodiscard]] bool acceptsFocus() const noexcept override {
        return *enabled;
    }

};

}
