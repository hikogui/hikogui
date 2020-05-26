// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
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

    std::string label = "<unknown>";

    Text::ShapedText labelShapedText;

    float toggle_height;
    float toggle_width;
    float toggle_x;
    float toggle_y;
    aarect toggle_rectangle;

    float slider_x;
    float slider_y;
    float slider_move;
    float slider_width;
    float slider_height;

    aarect label_rectangle;
    mat::T label_translate;
public:

    template<typename Value>
    ToggleWidget(Window &window, Widget *parent, Value &&value, std::string const label) noexcept :
        Widget(window, parent, vec{ssize(label) != 0 ? Theme::width : Theme::smallWidth, Theme::smallHeight}),
        value(150ms, std::forward<Value>(value), [this](auto...){ forceRedraw = true; }),
        label(std::move(label))
    {
    }
    ~ToggleWidget() {}

    ToggleWidget(const ToggleWidget &) = delete;
    ToggleWidget &operator=(const ToggleWidget &) = delete;
    ToggleWidget(ToggleWidget&&) = delete;
    ToggleWidget &operator=(ToggleWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override;
    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    void handleMouseEvent(GUI::MouseEvent const &event) noexcept override;
    void handleCommand(string_ltag command) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
    [[nodiscard]] bool acceptsFocus() const noexcept override {
        return enabled;
    }

};

}
