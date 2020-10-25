// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
#include "ToolbarWidget.hpp"
#include "GridLayoutWidget.hpp"
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "SystemMenuWidget.hpp"
#endif
#include "../GUI/utils.hpp"

namespace tt {

using namespace std;

WindowWidget::WindowWidget(Window &window, GridLayoutDelegate *delegate, Label title) noexcept :
    ContainerWidget(window, {}), title(std::move(title)), _content_delegate(delegate)
{
}

WindowWidget::~WindowWidget() {}

void WindowWidget::initialize() noexcept
{
    _toolbar = make_widget<ToolbarWidget>();

    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
        _toolbar->make_widget<SystemMenuWidget>(this->title.icon());
#endif
        _toolbar->make_widget<WindowTrafficLightsWidget, HorizontalAlignment::Right>();
    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        _toolbar->make_widget<WindowTrafficLightsWidget>();
    } else {
        tt_no_default();
    }

    _content = make_widget<GridLayoutWidget>(_content_delegate);
}

[[nodiscard]] bool WindowWidget::update_constraints() noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    if (ContainerWidget::update_constraints()) {
        ttlet toolbar_size = _toolbar->preferred_size();

        ttlet content_size = _content->preferred_size();
        p_preferred_size = intersect(
            max(content_size + toolbar_size._0y(), toolbar_size.x0()), interval_vec2::make_maximum(window.virtualScreenSize()));
        return true;
    } else {
        return false;
    }
}

bool WindowWidget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    need_layout |= std::exchange(request_relayout, false);
    if (need_layout) {
        ttlet toolbar_size = _toolbar->preferred_size();
        ttlet toolbar_height = toolbar_size.minimum().height();
        ttlet toolbar_rectangle = aarect{0.0f, rectangle().height() - toolbar_height, rectangle().width(), toolbar_height};
        _toolbar->set_layout_parameters(mat::T2{p_window_rectangle} * toolbar_rectangle, p_window_clipping_rectangle);

        ttlet content_size = _content->preferred_size();
        ttlet content_rectangle = aarect{0.0f, 0.0f, rectangle().width(), rectangle().height() - toolbar_height};
        _content->set_layout_parameters(mat::T2{p_window_rectangle} * content_rectangle, p_window_clipping_rectangle);
    }

    return ContainerWidget::update_layout(display_time_point, need_layout);
}

HitBox WindowWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);
    ttlet position = from_window_transform * window_position;

    constexpr float BORDER_WIDTH = 10.0f;

    ttlet is_on_left_edge = position.x() <= BORDER_WIDTH;
    ttlet is_on_right_edge = position.x() >= (rectangle().width() - BORDER_WIDTH);
    ttlet is_on_bottom_edge = position.y() <= BORDER_WIDTH;
    ttlet is_on_top_edge = position.y() >= (rectangle().height() - BORDER_WIDTH);

    ttlet is_on_bottom_left_corner = is_on_bottom_edge && is_on_left_edge;
    ttlet is_on_bottom_right_corner = is_on_bottom_edge && is_on_right_edge;
    ttlet is_on_top_left_corner = is_on_top_edge && is_on_left_edge;
    ttlet is_on_top_right_corner = is_on_top_edge && is_on_right_edge;

    auto r = HitBox{weak_from_this(), p_draw_layer};
    if (is_on_bottom_left_corner) {
        return {weak_from_this(), p_draw_layer, HitBox::Type::BottomLeftResizeCorner};
    } else if (is_on_bottom_right_corner) {
        return {weak_from_this(), p_draw_layer, HitBox::Type::BottomRightResizeCorner};
    } else if (is_on_top_left_corner) {
        return {weak_from_this(), p_draw_layer, HitBox::Type::TopLeftResizeCorner};
    } else if (is_on_top_right_corner) {
        return {weak_from_this(), p_draw_layer, HitBox::Type::TopRightResizeCorner};
    } else if (is_on_left_edge) {
        r.type = HitBox::Type::LeftResizeBorder;
    } else if (is_on_right_edge) {
        r.type = HitBox::Type::RightResizeBorder;
    } else if (is_on_bottom_edge) {
        r.type = HitBox::Type::BottomResizeBorder;
    } else if (is_on_top_edge) {
        r.type = HitBox::Type::TopResizeBorder;
    }

    if (
        (is_on_left_edge && left_resize_border_has_priority) ||
        (is_on_right_edge && right_resize_border_has_priority) ||
        (is_on_bottom_edge && bottom_resize_border_has_priority) ||
        (is_on_top_edge && top_resize_border_has_priority)
    ) {
        return r;
    }

    for (ttlet &child : children) {
        r = std::max(r, child->hitbox_test(window_position));
    }

    return r;
}

} // namespace tt
