
#include "ContainerWidget.hpp"
#include "../GUI/DrawContext.hpp"

namespace tt {

Widget &ContainerWidget::addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    current_address *= address;

    ttlet widget_ptr = childWidget.get();
    tt_assume(widget_ptr);

    children.push_back(std::move(childWidget));
    requestReconstrain = true;
    window.requestLayout = true;
    return *widget_ptr;
}

[[nodiscard]] bool ContainerWidget::updateConstraints() noexcept
{
    auto has_constrainted = Widget::updateConstraints();

    ttlet lock = std::scoped_lock(mutex);

    for (auto &&child: children) {
        has_constrainted |= child->updateConstraints();
    }

    return has_constrainted;
}

bool ContainerWidget::updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept
{
    auto has_laid_out = Widget::updateLayout(displayTimePoint, forceLayout);

    ttlet lock = std::scoped_lock(mutex);

    for (auto &&child: children) {
        has_laid_out |= child->updateLayout(displayTimePoint, forceLayout);
    }

    return has_laid_out;
}

void ContainerWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    auto lock = std::scoped_lock(mutex);

    auto childContext = drawContext;
    for (auto &child : children) {
        childContext.clippingRectangle = child->clippingRectangle();
        childContext.transform = child->toWindowTransform;

        // The default fill and border colors.
        ttlet childNestingLevel = child->nestingLevel();
        childContext.color = theme->borderColor(childNestingLevel);
        childContext.fillColor = theme->fillColor(childNestingLevel);

        if (*child->enabled) {
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

    Widget::draw(drawContext, displayTimePoint);
}

HitBox ContainerWidget::hitBoxTest(vec position) const noexcept
{
    auto lock = std::scoped_lock(mutex);

    auto r = rectangle().contains(position) ?
        HitBox{this, elevation} :
        HitBox{};

    for (ttlet &child : children) {
        r = std::max(r, child->hitBoxTest(position - child->offsetFromParent()));
    }
    return r;
}

std::vector<Widget *> ContainerWidget::childPointers(bool reverse) const noexcept {
    std::vector<Widget *> r;
    r.reserve(nonstd::ssize(children));
    for (ttlet &child: children) {
        r.push_back(child.get());
    }
    if (reverse) {
        std::reverse(r.begin(), r.end());
    }
    return r;
}

Widget *ContainerWidget::nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
{
    if (currentKeyboardWidget == nullptr && acceptsFocus()) {
        // The first widget that accepts focus.
        return const_cast<ContainerWidget *>(this);

    } else {
        bool found = false;

        for (auto *child: childPointers(reverse)) {
            if (found) {
                // Find the first focus accepting widget.
                if (auto *tmp = child->nextKeyboardWidget(nullptr, reverse)) {
                    return tmp;
                }

            } else if (child == currentKeyboardWidget) {
                found = true;

            } else {
                auto *tmp = child->nextKeyboardWidget(currentKeyboardWidget, reverse);
                if (tmp == foundWidgetPtr) {
                    // The current widget was found, but no next widget available in the child.
                    found = true;

                } else if (tmp) {
                    return tmp;
                }
            }
        }
        return found ? foundWidgetPtr : nullptr;
    }
}

}