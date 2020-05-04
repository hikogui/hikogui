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

void Widget::placeBelow(Widget const &rhs, float margin) const noexcept {
    window.addConstraint(this->box.top + margin == rhs.box.bottom);
}

void Widget::placeAbove(Widget const &rhs, float margin) const noexcept {
    window.addConstraint(this->box.bottom == rhs.box.top + margin);
}

void Widget::placeLeftOf(Widget const &rhs, float margin) const noexcept {
    window.addConstraint(this->box.right + margin == rhs.box.left);
}

void Widget::placeRightOf(Widget const &rhs, float margin) const noexcept {
    window.addConstraint(this->box.left == rhs.box.right + margin);
}

void Widget::shareTopEdgeWith(Widget const &parent, float margin, bool useContentArea) const noexcept {
    if (parent.content != nullptr && useContentArea) {
        shareTopEdgeWith(*parent.content, margin);
    } else {
        window.addConstraint(this->box.top + margin == parent.box.top);
    }
}

void Widget::shareBottomEdgeWith(Widget const &parent, float margin, bool useContentArea) const noexcept {
    if (parent.content != nullptr && useContentArea) {
        shareBottomEdgeWith(*parent.content, margin);
    } else {
        window.addConstraint(this->box.bottom - margin == parent.box.bottom);
    }
}

void Widget::shareLeftEdgeWith(Widget const &parent, float margin, bool useContentArea) const noexcept {
    if (parent.content != nullptr && useContentArea) {
        shareLeftEdgeWith(*parent.content, margin);
    } else {
        window.addConstraint(this->box.left - margin == parent.box.left);
    }
}

void Widget::shareRightEdgeWith(Widget const &parent, float margin, bool useContentArea) const noexcept {
    if (parent.content != nullptr && useContentArea) {
        shareRightEdgeWith(*parent.content, margin);
    } else {
        window.addConstraint(this->box.right + margin == parent.box.right);
    }
}

void Widget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    constexpr float elevationToDepth = 0.01f;

    let offset = box.currentOffset(elevation * elevationToDepth);

    auto childContext = drawContext;
    for (auto &child : children) {

        let childRectangle = child->box.currentRectangle();
        let childNestingLevel = child->nestingLevel();

        let relativeOffset = childRectangle.offset(child->elevation * elevationToDepth) - offset;
        let translation = mat::T(relativeOffset);

        childContext.clippingRectangle = expand(childRectangle, theme->margin);
        childContext.transform = translation * drawContext.transform;

        // The default fill and border colors.
        childContext.color = theme->borderColor(childNestingLevel);
        childContext.fillColor = theme->fillColor(childNestingLevel);

        if (child->enabled) {
            if (child->focus) {
                childContext.color = theme->accentColor;
            } else if (child->hover) {
                childContext.color = theme->borderColor(childNestingLevel + 1);
            }

            if (child->hover) {
                childContext.fillColor = theme->fillColor(childNestingLevel + 1);
            }

        } else {
            // Disabled, only the outline is shown.
            childContext.color = theme->borderColor(childNestingLevel - 1);
            childContext.fillColor = theme->fillColor(childNestingLevel - 1);
        }

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
