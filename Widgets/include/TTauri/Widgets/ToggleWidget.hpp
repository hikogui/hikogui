// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/Foundation/observer.hpp"
#include "TTauri/Foundation/animated.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

class ToggleWidget : public Widget {
protected:
    animated<observer<bool>> value;

    bool pressed = false;

    std::string label = "<unknown>";

    Text::ShapedText labelShapedText;
public:

    ToggleWidget(Window &window, Widget *parent, observed<bool> &value, std::string const label) noexcept;
    ~ToggleWidget() {}

    ToggleWidget(const ToggleWidget &) = delete;
    ToggleWidget &operator=(const ToggleWidget &) = delete;
    ToggleWidget(ToggleWidget&&) = delete;
    ToggleWidget &operator=(ToggleWidget &&) = delete;

    void draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(GUI::MouseEvent const &event) noexcept override;
    void handleCommand(string_ltag command) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept;
    [[nodiscard]] bool acceptsFocus() noexcept override {
        return enabled;
    }

};

}
