// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri {

Widget::Widget(Window &window, Widget *parent, vec defaultExtent) noexcept :
    window(window),
    parent(parent),
    elevation(parent ? parent->elevation + 1.0f : 0.0f),
    enabled(true, [this](auto...){ forceRedraw = true; })
{
    minimumExtent = defaultExtent;
    minimumWidthConstraint = window.addConstraint(width >= minimumExtent.width());
    minimumHeightConstraint = window.addConstraint(height >= minimumExtent.height());

    preferedExtent = defaultExtent;
    preferedWidthConstraint = window.addConstraint(width >= preferedExtent.width(), rhea::strength::strong());
    preferedHeightConstraint = window.addConstraint(height >= preferedExtent.height(), rhea::strength::strong());
}

Widget::~Widget()
{
    window.removeConstraint(minimumWidthConstraint);
    window.removeConstraint(minimumHeightConstraint);
    window.removeConstraint(preferedWidthConstraint);
    window.removeConstraint(preferedHeightConstraint);
}

GUIDevice *Widget::device() const noexcept
{
    auto device = window.device;
    ttauri_assert(device);
    return device;
}

void Widget::setMinimumExtent(vec newMinimumExtent) noexcept
{
    auto lock = std::scoped_lock(mutex);

    if (newMinimumExtent != minimumExtent) {
        minimumExtent = newMinimumExtent;

        minimumWidthConstraint = window.replaceConstraint(
            minimumWidthConstraint,
            width >= minimumExtent.width()
        );

        minimumHeightConstraint = window.replaceConstraint(
            minimumHeightConstraint,
            height >= minimumExtent.height()
        );
    }
}

void Widget::setMinimumExtent(float _width, float _height) noexcept
{
    setMinimumExtent(vec{_width, _height});
}

void Widget::setPreferedExtent(vec newPreferedExtent) noexcept
{
    auto lock = std::scoped_lock(mutex);

    if (newPreferedExtent != preferedExtent) {
        preferedExtent = newPreferedExtent;

        preferedWidthConstraint = window.replaceConstraint(
            preferedWidthConstraint,
            width >= preferedExtent.width(),
            rhea::strength::weak()
        );

        preferedHeightConstraint = window.replaceConstraint(
            preferedHeightConstraint,
            height >= preferedExtent.height(),
            rhea::strength::weak()
        );
    }
}

void Widget::setFixedExtent(vec newFixedExtent) noexcept
{
    auto lock = std::scoped_lock(mutex);

    ttauri_assert(newFixedExtent.width() == 0.0f || newFixedExtent.width() >= minimumExtent.width());
    ttauri_assert(newFixedExtent.height() == 0.0f || newFixedExtent.height() >= minimumExtent.height());

    if (newFixedExtent != fixedExtent) {
        if (fixedExtent.width() != 0.0f) {
            window.removeConstraint(fixedWidthConstraint);
        }
        if (fixedExtent.height() != 0.0f) {
            window.removeConstraint(fixedHeightConstraint);
        }
        fixedExtent = newFixedExtent;
        if (fixedExtent.width() != 0.0f) {
            fixedWidthConstraint = window.addConstraint(width == fixedExtent.width());
        }
        if (fixedExtent.height() != 0.0f) {
            fixedHeightConstraint = window.addConstraint(height == fixedExtent.height());
        }
    }
}

void Widget::setFixedHeight(float _height) noexcept
{
    setFixedExtent(vec{0.0f, _height});
}

void Widget::setFixedWidth(float _width) noexcept
{
    setFixedExtent(vec{_width, 0.0f});
}

rhea::constraint Widget::placeBelow(Widget const &rhs, float margin) const noexcept {
    return window.addConstraint(this->top + margin == rhs.bottom);
}

rhea::constraint Widget::placeAbove(Widget const &rhs, float margin) const noexcept {
    return window.addConstraint(this->bottom == rhs.top + margin);
}

rhea::constraint Widget::placeLeftOf(Widget const &rhs, float margin) const noexcept {
    return window.addConstraint(this->right + margin == rhs.left);
}

rhea::constraint Widget::placeRightOf(Widget const &rhs, float margin) const noexcept {
    return window.addConstraint(this->left == rhs.right + margin);
}

rhea::constraint Widget::placeAtTop(float margin) const noexcept {
    return window.addConstraint(this->top + margin == parent->top);
}

rhea::constraint Widget::placeAtBottom(float margin) const noexcept {
    return window.addConstraint(this->bottom - margin == parent->bottom);
}

rhea::constraint Widget::placeLeft(float margin) const noexcept {
    return window.addConstraint(this->left - margin == parent->left);
}

rhea::constraint Widget::placeRight(float margin) const noexcept {
    return window.addConstraint(this->right + margin == parent->right);
}

int Widget::needs(hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto lock = std::scoped_lock(window.widgetSolverMutex);
    int needs = 0;

    auto newExtent = i32x2_t{width.value(), height.value()};
    needs |= static_cast<int>(newExtent != i32_extent());
    setExtent(newExtent);

    auto newOffsetFromWindow = i32x2_t{left.value(), bottom.value()};
    needs |= static_cast<int>(newOffsetFromWindow != i32_offsetFromWindow());
    setOffsetFromWindow(newOffsetFromWindow);

    needs |= static_cast<int>(
        forceLayout.exchange(false, std::memory_order::memory_order_relaxed)
    );

    needs = (needs << 1) | needs;    
    needs |= static_cast<int>(
        forceRedraw.exchange(false, std::memory_order::memory_order_relaxed)
    );

    return needs;
}

void Widget::layout(hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto lock = std::scoped_lock(mutex);

    setOffsetFromParent(
        parent ?
            offsetFromWindow() - parent->offsetFromWindow():
            offsetFromWindow()
    );
        
    fromWindowTransform = mat::T(-offsetFromWindow().x(), -offsetFromWindow().y(), -z());
    toWindowTransform = mat::T(offsetFromWindow().x(), offsetFromWindow().y(), z());

    forceRedraw = true;
}

Widget &Widget::addWidget(Alignment, std::unique_ptr<Widget> childWidget) noexcept {
    let widget_ptr = childWidget.get();
    ttauri_assume(widget_ptr);

    auto lock = std::scoped_lock(mutex);
    children.push_back(std::move(childWidget));
    window.forceLayout = true;
    return *widget_ptr;
}

int Widget::layoutChildren(hires_utc_clock::time_point displayTimePoint, bool force) noexcept
{
    auto lock = std::scoped_lock(mutex);

    auto total_need = 0;

    for (auto &&child: children) {
        let child_need = child->needs(displayTimePoint);
        total_need |= child_need;

        if (force || child_need >= 2) {
            child->layout(displayTimePoint);
        }

        total_need |= child->layoutChildren(displayTimePoint, force);
    }

    return total_need;
}

void Widget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto lock = std::scoped_lock(mutex);

    auto childContext = drawContext;
    for (auto &child : children) {
        childContext.clippingRectangle = child->clippingRectangle();
        childContext.transform = child->toWindowTransform;

        // The default fill and border colors.
        let childNestingLevel = child->nestingLevel();
        childContext.color = theme->borderColor(childNestingLevel);
        childContext.fillColor = theme->fillColor(childNestingLevel);

        if (child->enabled) {
            if (child->focus && window.active) {
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
    auto lock = std::scoped_lock(mutex);

    if (command == "gui.widget.next"_ltag) {
        window.updateToNextKeyboardTarget(this);
    } else if (command == "gui.widget.prev"_ltag) {
        window.updateToPrevKeyboardTarget(this);
    }
}

HitBox Widget::hitBoxTest(vec position) const noexcept
{
    auto lock = std::scoped_lock(mutex);

    auto r = rectangle().contains(position) ?
        HitBox{this, elevation} :
        HitBox{};

    for (let &child : children) {
        r = std::max(r, child->hitBoxTest(position - child->offsetFromParent()));
    }
    return r;
}

}
