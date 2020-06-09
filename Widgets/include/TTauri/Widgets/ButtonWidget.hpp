// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include <rhea/constraint.hpp>
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri {

class ButtonWidget : public Widget {
protected:
    bool value = false;
    bool pressed = false;

    std::string label = "<unknown>";

    Text::ShapedText labelShapedText;
    mat::T textTranslate;
public:

    ButtonWidget(Window &window, Widget *parent, std::string const label) noexcept;
    ~ButtonWidget();

    ButtonWidget(const ButtonWidget &) = delete;
    ButtonWidget &operator=(const ButtonWidget &) = delete;
    ButtonWidget(ButtonWidget&&) = delete;
    ButtonWidget &operator=(ButtonWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(MouseEvent const &event) noexcept override;
    void handleCommand(string_ltag command) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
    [[nodiscard]] bool acceptsFocus() const noexcept override {
        return enabled;
    }

};

}
