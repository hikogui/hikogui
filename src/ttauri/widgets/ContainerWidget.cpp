
#include "ContainerWidget.hpp"
#include "../GUI/DrawContext.hpp"

namespace tt {

Widget &ContainerWidget::addWidget(std::unique_ptr<Widget> childWidget) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    children.push_back(std::move(childWidget));
    request_reconstrain = true;
    window.requestLayout = true;

    ttlet widget_ptr = children.back().get();
    tt_assume(widget_ptr);
    return *widget_ptr;
}

[[nodiscard]] bool ContainerWidget::update_constraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto has_constrainted = Widget::update_constraints();

    for (auto &&child : children) {
        ttlet child_lock = std::scoped_lock(child->mutex);
        has_constrainted |= child->update_constraints();
    }

    return has_constrainted;
}

bool ContainerWidget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto need_redraw = need_layout |= std::exchange(request_relayout, false);
    for (auto &&child : children) {
        ttlet child_lock = std::scoped_lock(child->mutex);
        need_redraw |= child->update_layout(display_time_point, need_layout);
    }

    return Widget::update_layout(display_time_point, need_layout) || need_redraw;
}


void ContainerWidget::draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    for (auto &child : children) {
        ttlet child_lock = std::scoped_lock(child->mutex);
        child->draw(child->make_draw_context(context), display_time_point);
    }

    Widget::draw(std::move(context), display_time_point);
}

HitBox ContainerWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    auto r = HitBox{};
    for (ttlet &child : children) {
        r = std::max(r, child->hitbox_test(window_position));
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

Widget const *ContainerWidget::next_keyboard_widget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    // If currentKeyboardWidget is nullptr, then we need to find the first widget that accepts focus.
    auto found = (currentKeyboardWidget == nullptr);

    // The container widget itself accepts focus.
    if (found && !reverse && accepts_focus()) {
        return this;
    }

    for (auto *child : childPointers(reverse)) {
        if (found) {
            // Find the first focus accepting widget.
            if (auto *tmp = child->next_keyboard_widget(nullptr, reverse)) {
                return tmp;
            }

        } else if (child == currentKeyboardWidget) {
            found = true;

        } else {
            auto *tmp = child->next_keyboard_widget(currentKeyboardWidget, reverse);
            if (tmp == currentKeyboardWidget) {
                // The current widget was found, but no next widget available in the child.
                found = true;

            } else if (tmp) {
                return tmp;
            }
        }
    }

    // The container widget itself accepts focus.
    if (found && reverse && accepts_focus()) {
        return this;
    }

    return found ? currentKeyboardWidget : nullptr;
}

} // namespace tt