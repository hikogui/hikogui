
#include "ContainerWidget.hpp"
#include "../GUI/DrawContext.hpp"

namespace tt {

Widget &ContainerWidget::addWidget(std::unique_ptr<Widget> childWidget) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    children.push_back(std::move(childWidget));
    requestConstraint = true;
    window.requestLayout = true;

    ttlet widget_ptr = children.back().get();
    tt_assume(widget_ptr);
    return *widget_ptr;
}

[[nodiscard]] bool ContainerWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto has_constrainted = Widget::updateConstraints();

    for (auto &&child : children) {
        ttlet child_lock = std::scoped_lock(child->mutex);
        has_constrainted |= child->updateConstraints();
    }

    return has_constrainted;
}

bool ContainerWidget::updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto need_redraw = need_layout |= std::exchange(requestLayout, false);
    for (auto &&child : children) {
        ttlet child_lock = std::scoped_lock(child->mutex);
        need_redraw |= child->updateLayout(display_time_point, need_layout);
    }

    return Widget::updateLayout(display_time_point, need_layout) || need_redraw;
}


void ContainerWidget::draw(DrawContext const &context, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    for (auto &child : children) {
        ttlet child_lock = std::scoped_lock(child->mutex);
        child->draw(child->makeDrawContext(context), displayTimePoint);
    }

    Widget::draw(context, displayTimePoint);
}

HitBox ContainerWidget::hitBoxTest(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    ttlet position = fromWindowTransform * window_position;

    auto r = rectangle().contains(position) ? HitBox{this, elevation} : HitBox{};

    for (ttlet &child : children) {
        r = std::max(r, child->hitBoxTest(window_position));
    }
    return r;
}

std::vector<Widget *> ContainerWidget::childPointers(bool reverse) const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    std::vector<Widget *> r;
    r.reserve(std::ssize(children));
    for (ttlet &child : children) {
        r.push_back(child.get());
    }
    if (reverse) {
        std::reverse(r.begin(), r.end());
    }
    return r;
}

Widget const *ContainerWidget::nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    // If currentKeyboardWidget is nullptr, then we need to find the first widget that accepts focus.
    auto found = (currentKeyboardWidget == nullptr);

    // The container widget itself accepts focus.
    if (found && !reverse && acceptsFocus()) {
        return this;
    }

    for (auto *child : childPointers(reverse)) {
        if (found) {
            // Find the first focus accepting widget.
            if (auto *tmp = child->nextKeyboardWidget(nullptr, reverse)) {
                return tmp;
            }

        } else if (child == currentKeyboardWidget) {
            found = true;

        } else {
            auto *tmp = child->nextKeyboardWidget(currentKeyboardWidget, reverse);
            if (tmp == currentKeyboardWidget) {
                // The current widget was found, but no next widget available in the child.
                found = true;

            } else if (tmp) {
                return tmp;
            }
        }
    }

    // The container widget itself accepts focus.
    if (found && reverse && acceptsFocus()) {
        return this;
    }

    return found ? currentKeyboardWidget : nullptr;
}

} // namespace tt