// Copyright 2019 Pokitec
// All rights reserved.

#include "ToolbarWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
#include "../GUI/utils.hpp"
#include "../interval.hpp"
#include <cmath>
#include <ranges>

namespace tt {

using namespace std;

ToolbarWidget::ToolbarWidget(Window &window, std::shared_ptr<widget> parent) noexcept : ContainerWidget(window, parent)
{
    if (parent) {
        // The toolbar widget does draw itself.
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        _draw_layer = parent->draw_layer() + 1.0f;
        _semantic_layer = parent->semantic_layer() + 1;
    }
}

std::shared_ptr<widget> ToolbarWidget::add_widget(HorizontalAlignment alignment, std::shared_ptr<widget> widget) noexcept
{
    auto tmp = ContainerWidget::add_widget(std::move(widget));
    switch (alignment) {
        using enum HorizontalAlignment;
    case Left: left_children.push_back(tmp); break;
    case Right: right_children.push_back(tmp); break;
    default: tt_no_default();
    }

    return tmp;
}

void ToolbarWidget::updateConstraintsForChild(
    widget const &child,
    ssize_t index,
    relative_base_line &shared_base_line,
    finterval &shared_height) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    layout.update(index, child.preferred_size().width(), child.width_resistance(), child.margin(), relative_base_line{});

    shared_base_line = std::max(shared_base_line, child.preferred_base_line());
    shared_height = intersect(shared_height, child.preferred_size().height() + child.margin() * 2.0f);
}

[[nodiscard]] bool ToolbarWidget::update_constraints() noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    if (ContainerWidget::update_constraints()) {
        auto shared_base_line = relative_base_line{VerticalAlignment::Middle, 0.0f, 100};
        auto shared_height = finterval{};

        layout.clear();
        layout.reserve(std::ssize(left_children) + 1 + std::ssize(right_children));

        ssize_t index = 0;
        for (ttlet &child : left_children) {
            updateConstraintsForChild(*child, index++, shared_base_line, shared_height);
        }

        // Add a space between the left and right widgets.
        layout.update(
            index++, finterval{Theme::width, std::numeric_limits<float>::max()}, ranged_int<3>{1}, 0.0f, relative_base_line{});

        for (ttlet &child : std::views::reverse(right_children)) {
            updateConstraintsForChild(*child, index++, shared_base_line, shared_height);
        }

        tt_assume(index == std::ssize(left_children) + 1 + std::ssize(right_children));
        _preferred_size = {layout.extent(), finterval{shared_height.minimum()}};
        _preferred_base_line = shared_base_line;
        return true;
    } else {
        return false;
    }
}

void ToolbarWidget::updateLayoutForChild(widget &child, ssize_t index) const noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    ttlet[child_x, child_width] = layout.get_offset_and_size(index++);

    ttlet child_rectangle = aarect{
        rectangle().x() + child_x, rectangle().y() + child.margin(), child_width, rectangle().height() - child.margin() * 2.0f};

    ttlet child_window_rectangle = mat::T2{_window_rectangle} * child_rectangle;

    child.set_layout_parameters(child_window_rectangle, _window_clipping_rectangle, _window_base_line);
}

bool ToolbarWidget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
    if (need_layout) {
        layout.update_layout(rectangle().width());

        ssize_t index = 0;
        for (ttlet &child : left_children) {
            updateLayoutForChild(*child, index++);
        }

        // Skip over the cell between left and right children.
        index++;

        for (ttlet &child : std::views::reverse(right_children)) {
            updateLayoutForChild(*child, index++);
        }

        tt_assume(index == std::ssize(left_children) + 1 + std::ssize(right_children));
    }
    return ContainerWidget::update_layout(display_time_point, need_layout);
}

void ToolbarWidget::draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    context.drawFilledQuad(rectangle());
    ContainerWidget::draw(std::move(context), display_time_point);
}

HitBox ToolbarWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    auto r = HitBox{};

    if (_window_clipping_rectangle.contains(window_position)) {
        r = HitBox{weak_from_this(), _draw_layer, HitBox::Type::MoveArea};
    }

    for (ttlet &child : children) {
        r = std::max(r, child->hitbox_test(window_position));
    }
    return r;
}

} // namespace tt
