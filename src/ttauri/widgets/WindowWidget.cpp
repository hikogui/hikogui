// Copyright 2019 Pokitec
// All rights reserved.

#include "WindowWidget.hpp"
#include "WindowTrafficLightsWidget.hpp"
#include "ToolbarWidget.hpp"
#include "GridWidget.hpp"
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "SystemMenuWidget.hpp"
#endif
#include "../GUI/utils.hpp"

namespace tt {

using namespace std;

WindowWidget::WindowWidget(Window &window, GridWidgetDelegate *delegate, Label title) noexcept :
    ContainerWidget(window, nullptr), title(std::move(title))
{
    _toolbar = &makeWidget<ToolbarWidget>();

    if constexpr (Theme::operatingSystem == OperatingSystem::Windows) {
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
        _toolbar->makeWidget<SystemMenuWidget>(this->title.icon());
#endif
        _toolbar->makeWidget<WindowTrafficLightsWidget, HorizontalAlignment::Right>();
    } else if constexpr (Theme::operatingSystem == OperatingSystem::MacOS) {
        _toolbar->makeWidget<WindowTrafficLightsWidget>();
    } else {
        tt_no_default();
    }

    _content = &makeWidget<GridWidget>(delegate);
}

WindowWidget::~WindowWidget() {}

[[nodiscard]] bool WindowWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ContainerWidget::updateConstraints()) {
        ttlet toolbar_lock = std::scoped_lock(_toolbar->mutex);
        ttlet toolbar_size = _toolbar->preferred_size();

        ttlet content_lock = std::scoped_lock(_content->mutex);
        ttlet content_size = _content->preferred_size();
        _preferred_size = intersect(
            max(content_size + toolbar_size._0y(), toolbar_size.x0()),
            interval_vec2::make_maximum(window.virtualScreenSize())   
        );
        return true;
    } else {
        return false;
    }
}

bool WindowWidget::updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    need_layout |= std::exchange(requestLayout, false);
    if (need_layout) {
        ttlet toolbar_lock = std::scoped_lock(_toolbar->mutex);
        ttlet toolbar_size = _toolbar->preferred_size();
        ttlet toolbar_height = toolbar_size.minimum().height();
        ttlet toolbar_rectangle = aarect{0.0f, rectangle().height() - toolbar_height, rectangle().width(), toolbar_height};
        _toolbar->set_layout_parameters(mat::T2{_window_rectangle} * toolbar_rectangle, _window_clipping_rectangle);

        ttlet content_lock = std::scoped_lock(_content->mutex);
        ttlet content_size = _content->preferred_size();
        ttlet content_rectangle = aarect{0.0f, 0.0f, rectangle().width(), rectangle().height() - toolbar_height};
        _content->set_layout_parameters(mat::T2{_window_rectangle} * content_rectangle, _window_clipping_rectangle);
    }

    return ContainerWidget::updateLayout(display_time_point, need_layout);
}

HitBox WindowWidget::hitBoxTest(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    ttlet position = fromWindowTransform * window_position;

    constexpr float BORDER_WIDTH = 10.0f;

    auto r = HitBox{this, _draw_layer};

    if (position.x() <= BORDER_WIDTH) {
        if (position.y() <= BORDER_WIDTH) {
            r.type = HitBox::Type::BottomLeftResizeCorner;
        } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopLeftResizeCorner;
        } else {
            r.type = HitBox::Type::LeftResizeBorder;
        }

    } else if (position.x() >= (rectangle().width() - BORDER_WIDTH)) {
        if (position.y() <= BORDER_WIDTH) {
            r.type = HitBox::Type::BottomRightResizeCorner;
        } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopRightResizeCorner;
        } else {
            r.type = HitBox::Type::RightResizeBorder;
        }

    } else if (position.y() <= BORDER_WIDTH) {
        r.type = HitBox::Type::BottomResizeBorder;

    } else if (position.y() >= (rectangle().height() - BORDER_WIDTH)) {
        r.type = HitBox::Type::TopResizeBorder;
    }

    if (r.type != HitBox::Type::Outside) {
        // Resize corners need to override anything else, so that it is
        // always possible to resize a window.
        return r;
    }

    for (ttlet &child : children) {
        r = std::max(r, child->hitBoxTest(window_position));
    }

    return r;
}

} // namespace tt
