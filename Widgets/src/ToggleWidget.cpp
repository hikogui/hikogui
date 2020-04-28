// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ToggleWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include "TTauri/Foundation/math.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;

ToggleWidget::ToggleWidget(Window &window, Widget *parent, observed<bool> &value, std::string const label) noexcept :
    Widget(window, parent), value(250ms, value, [this](bool){ ++this->renderTrigger; }), label(std::move(label))
{
}

void ToggleWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    constexpr float toggle_height = 20.0f;
    constexpr float toggle_half_height = toggle_height / 2;
    constexpr float toggle_width = toggle_height t* 3.0;
    constexpr float toggle_move = toggle_width - toggle_height;
    constexpr float label_offset_x = toggle_width + toggle_height * 0.5;
    float toggle_offset_y = (box.height() - toggle_height) * 0.5;
    float label_width = box.width() - label_offset_x;
    let onLabelRectangle = rect{label_offset_x, 0.0, label_width, box.height()};
    let labelRectangle = rect{label_offset_x, 0.0, label_width, box.height()};

    auto context = drawContext;

    // Prepare labels.
    if (renderTrigger.check(displayTimePoint) >= 2) {
        OnLabelShapedText = ShapedText(onLabel, theme->smallLabelStyle, HorizontalAlignment::Left, onLabelRectangle.width());
        window.device->SDFPipeline->prepareAtlas(labelShapedText);

        labelShapedText = ShapedText(label, theme->labelStyle, HorizontalAlignment::Left, labelWidth);
        window.device->SDFPipeline->prepareAtlas(labelShapedText);
    }
    auto onLabelOffset = onLabelRectangle.align(onLabelShapedText.extent, Alignment::MiddleLeft);
    auto labelOffset = labelRectangle.align(labelShapedText.extent, Alignment::MiddleLeft);

    // Prepare animation values.
    let [animation_progress, curr_value] = value.animation_tick(displayTimePoint);
    if (animation_progress < 1.0) {
        ++renderTrigger;
    }

    // Outside oval.
    if (focus) {
        context.color = theme->accentColor;
    } else {
        context.color = theme->fillColor(nestingLevel() + 1);
    }
    if (hover) {
        context,fillColor = theme->fillColor(nestingLevel());
    } else {
        context,fillColor = theme->fillColor(nestingLevel() - 1);
    }
    context.lineWidth = theme->toggleBorderWidth;
    context.cornerShapes = vec{toggle_half_height};
    context.drawBox(expand(rect{
        0.0, toggle_offset_y,
        toggle_width, toggle_height
    }, 0.5));

    // Inside circle
    context.color = theme->fillColor(nestingLevel() + 1);
    if (hover) {
        context.fillColor = mix(curr_value, theme->fillColor(nestingLevel() + 1), theme->accentColor);
    } else if (pressed) {
        context.fillColor = mix(curr_value, theme->accentColor, theme->fillColor(nestingLevel() + 1));
    } else {
        context.fillColor = mix(curr_value, theme->fillColor(nestingLevel()), theme->accentColor);
    }
    context.drawBox(expand(rect{
        curr_value * toggle_move, toggle_offset_y,
        toggle_height, toggle_height
    }, 0.5));


    context.transform = context.transform * mat::T{textOffset.z(0.001f)};
    context.drawText(labelShapedText);

    Widget::draw(drawContext, displayTimePoint);
}

void ToggleWidget::handleCommand(string_ltag command) noexcept {
    if (!enabled) {
        return;
    }

    if (command == "gui.activate"_ltag) {
        if (assign_and_compare(value, !value)) {
            ++renderTrigger;
        }
    }
    Widget::handleCommand(command);
}

void ToggleWidget::handleMouseEvent(GUI::MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (enabled) {
        if (assign_and_compare(pressed, static_cast<bool>(event.down.leftButton))) {
            ++renderTrigger;
        }

        if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            handleCommand("gui.activate"_ltag);
        }
    }
}

HitBox ToggleWidget::hitBoxTest(vec position) noexcept
{
    if (box.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
