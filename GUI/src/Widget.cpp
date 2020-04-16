// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri::GUI::Widgets {

Widget::Widget(Window &window, Widget *parent) noexcept :
    window(window), parent(parent), renderTrigger(&window.renderTrigger), elevation(parent ? parent->elevation + 1.0f : 0.0f) {}

Device *Widget::device() const noexcept
{
    auto device = window.device;
    ttauri_assert(device);
    return device;
}

void Widget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    constexpr float elevationToDepth = 0.01f;

    let offset = box.currentOffset(elevation * elevationToDepth);

    auto childContext = drawContext;
    for (auto &child : children) {
        let childRectangle = child->box.currentRectangle();

        let relativeOffset = childRectangle.offset(child->elevation * elevationToDepth) - offset;
        let translation = mat::T(relativeOffset);

        childContext.clippingRectangle = childRectangle;
        childContext.transform = translation * drawContext.transform;
        child->draw(childContext, displayTimePoint);
    }
}

void Widget::handleCommand(string_ltag command) noexcept {
    if (command == "gui.widget.next"_ltag) {
        window.updateToNextKeyboardTarget(this);
    } else if (command == "gui.widget.prev"_ltag) {
        window.updateToPrevKeyboardTarget(this);
    }
}

HitBox Widget::hitBoxTest(vec position) noexcept
{
    auto r = box.contains(position) ?
        HitBox{this, elevation} :
        HitBox{};

    for (auto& widget : children) {
        r = std::max(r, widget->hitBoxTest(position));
    }
    return r;
}

}
