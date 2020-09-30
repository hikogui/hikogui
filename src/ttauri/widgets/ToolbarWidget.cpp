// Copyright 2019 Pokitec
// All rights reserved.

#include "ToolbarWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
#include "ToolbarButtonWidget.hpp"
#include "../GUI/utils.hpp"
#include "../interval.hpp"
#include <cmath>
#include <ranges>

namespace tt {

using namespace std;

ToolbarWidget::ToolbarWidget(Window &window, Widget *parent) noexcept : ContainerWidget(window, parent) {}

Widget &ToolbarWidget::addWidget(HorizontalAlignment alignment, std::unique_ptr<Widget> childWidget) noexcept
{
    auto &widget = ContainerWidget::addWidget(std::move(childWidget));
    switch (alignment) {
        using enum HorizontalAlignment;
    case Left: left_children.push_back(&widget); break;
    case Right: right_children.push_back(&widget); break;
    default: tt_no_default;
    }

    return widget;
}

[[nodiscard]] bool ToolbarWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ContainerWidget::updateConstraints()) {
        auto width = finterval{0.0f};
        auto height = finterval{};
        child_base_line = relative_base_line{VerticalAlignment::Middle, 0.0f, 100};
        auto prev_right_margin = 0.0f;
        for (ttlet &child : left_children) {
            ttlet child_lock = std::scoped_lock(child->mutex);
            ttlet _margin = std::max(prev_right_margin, child->margin);
            prev_right_margin = child->margin;

            width += _margin;
            width += child->preferred_size().width();
            height = intersect(height, child->preferred_size().height() + child->margin * 2.0f);
            child_base_line = std::max(child_base_line, child->preferred_base_line());
        }

        for (ttlet &child : views::reverse(right_children)) {
            ttlet child_lock = std::scoped_lock(child->mutex);
            ttlet _margin = std::max(prev_right_margin, child->margin);
            prev_right_margin = child->margin;

            width += _margin;
            width += child->preferred_size().width();
            height = intersect(height, child->preferred_size().height() + child->margin * 2.0f);
            child_base_line = std::max(child_base_line, child->preferred_base_line());
        }

        // Add right hand margin for the last child added.
        width += prev_right_margin;

        _preferred_size = {width, height};
        return true;
    } else {
        return false;
    }
}

bool ToolbarWidget::updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    need_layout |= std::exchange(requestLayout, false);
    if (need_layout) {
        auto extra_width = rectangle().width() - _preferred_size.width().minimum();

        auto x = rectangle().left();
        auto prev_right_margin = 0.0f;
        ttlet base_line_position = child_base_line.position(window_rectangle().bottom(), window_rectangle().top()); 
        for (ttlet &child : left_children) {
            ttlet child_lock = std::scoped_lock(child->mutex);

            prev_right_margin = child->margin;
            ttlet _margin = std::max(prev_right_margin, child->margin);

            x += _margin;
            ttlet width = std::clamp(
                child->preferred_size().width().maximum(),
                child->preferred_size().width().minimum(),
                child->preferred_size().width().minimum() + extra_width);

            if (width > child->preferred_size().width().minimum()) {
                extra_width -= (width - child->preferred_size().width().minimum());
            }

            ttlet child_rectangle = aarect{x, child->margin, width, rectangle().height() - child->margin * 2.0f};
            child->set_window_rectangle(mat::T2(window_rectangle()) * child_rectangle);
            child->set_window_base_line(base_line_position);

            x += width;
        }

        auto prev_left_margin = 0.0f;
        x = rectangle().right();
        for (ttlet &child : right_children) {
            ttlet child_lock = std::scoped_lock(child->mutex);

            prev_left_margin = child->margin;
            ttlet _margin = std::max(prev_left_margin, child->margin);

            x -= _margin;
            ttlet width = std::clamp(
                child->preferred_size().width().maximum(),
                child->preferred_size().width().minimum(),
                child->preferred_size().width().minimum() + extra_width);

            if (width > child->preferred_size().width().minimum()) {
                extra_width -= (width - child->preferred_size().width().minimum());
            }

            x -= width;
            ttlet child_rectangle = aarect{x, child->margin, width, rectangle().height() - child->margin * 2.0f};
            child->set_window_rectangle(mat::T2(window_rectangle()) * child_rectangle);
            child->set_window_base_line(base_line_position);
        }
    }
    return ContainerWidget::updateLayout(display_time_point, need_layout);
}

void ToolbarWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto context = drawContext;
    context.drawFilledQuad(rectangle());
    ContainerWidget::draw(drawContext, displayTimePoint);
}

HitBox ToolbarWidget::hitBoxTest(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    ttlet position = fromWindowTransform * window_position;

    auto r = rectangle().contains(position) ? HitBox{this, elevation, HitBox::Type::MoveArea} : HitBox{};

    for (ttlet &child : children) {
        r = std::max(r, child->hitBoxTest(window_position));
    }
    return r;
}

} // namespace tt
