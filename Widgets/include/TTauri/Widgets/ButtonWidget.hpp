// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/ControlWidget.hpp"
#include <rhea/constraint.hpp>
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

class ButtonWidget : public ControlWidget {
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

    [[nodiscard]] void layout() noexcept override;

    void draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(GUI::MouseEvent const &event) noexcept override;
    void handleCommand(string_ltag command) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept;
    [[nodiscard]] bool acceptsFocus() noexcept override {
        return enabled;
    }

};

}
