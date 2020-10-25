
#include "ContainerWidget.hpp"
#include "../GUI/DrawContext.hpp"

namespace tt {

std::shared_ptr<Widget> ContainerWidget::add_widget(std::shared_ptr<Widget> widget) noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    tt_assume(widget->parent.lock().get() == this);
    children.push_back(widget);
    request_reconstrain = true;
    window.requestLayout = true;
    return widget;
}

[[nodiscard]] bool ContainerWidget::update_constraints() noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    auto has_constrainted = Widget::update_constraints();

    for (auto &&child : children) {
        tt_assume(child->parent.lock() == shared_from_this());
        has_constrainted |= child->update_constraints();
    }

    return has_constrainted;
}

bool ContainerWidget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    auto need_redraw = need_layout |= std::exchange(request_relayout, false);
    for (auto &&child : children) {
        tt_assume(child->parent.lock().get() == this);
        need_redraw |= child->update_layout(display_time_point, need_layout);
    }

    return Widget::update_layout(display_time_point, need_layout) || need_redraw;
}


void ContainerWidget::draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    for (auto &child : children) {
        tt_assume(child->parent.lock().get() == this);
        child->draw(child->make_draw_context(context), display_time_point);
    }

    Widget::draw(std::move(context), display_time_point);
}

HitBox ContainerWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    auto r = HitBox{};
    for (ttlet &child : children) {
        tt_assume(child->parent.lock().get() == this);
        r = std::max(r, child->hitbox_test(window_position));
    }
    return r;
}

std::shared_ptr<Widget> ContainerWidget::next_keyboard_widget(std::shared_ptr<Widget> const &current_keyboard_widget, bool reverse) const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    // If current_keyboard_widget is empty, then we need to find the first widget that accepts focus.
    auto found = !current_keyboard_widget;

    // The container widget itself accepts focus.
    if (found && !reverse && accepts_focus()) {
        return std::const_pointer_cast<Widget>(shared_from_this());
    }

    ssize_t first = reverse ? ssize(children) - 1 : 0;
    ssize_t last = reverse ? -1 : ssize(children);
    ssize_t step = reverse ? -1 : 1;
    for (ssize_t i = first; i != last; i += step) {
        auto &&child = children[i];

        if (found) {
            // Find the first focus accepting widget.
            if (auto tmp = child->next_keyboard_widget({}, reverse)) {
                return tmp;
            }

        } else if (child == current_keyboard_widget) {
            found = true;

        } else {
            auto tmp = child->next_keyboard_widget(current_keyboard_widget, reverse);
            if (tmp == current_keyboard_widget) {
                // The current widget was found, but no next widget available in the child.
                found = true;

            } else if (tmp) {
                return tmp;
            }
        }
    }

    // The container widget itself accepts focus.
    if (found && reverse && accepts_focus()) {
        return std::const_pointer_cast<Widget>(shared_from_this());
    }

    return found ? current_keyboard_widget : std::shared_ptr<Widget>{};
}

} // namespace tt