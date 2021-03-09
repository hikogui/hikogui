// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "window_widget.hpp"
#include "window_traffic_lights_widget.hpp"
#include "toolbar_widget.hpp"
#include "grid_layout_widget.hpp"
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "system_menu_widget.hpp"
#endif
#include "../GUI/utils.hpp"

namespace tt {

using namespace std;

window_widget::window_widget(gui_window &window, std::weak_ptr<grid_layout_delegate> const &delegate, label title) noexcept :
    abstract_container_widget(window, {}), title(std::move(title)), _content_delegate(delegate)
{
}

window_widget::~window_widget() {}

void window_widget::init() noexcept
{
    _toolbar = make_widget<toolbar_widget>();

    if constexpr (theme::global->operatingSystem == OperatingSystem::Windows) {
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
        _toolbar->make_widget<system_menu_widget>(this->title.icon());
#endif
        _toolbar->make_widget<window_traffic_lights_widget, horizontal_alignment::right>();
    } else if constexpr (theme::global->operatingSystem == OperatingSystem::MacOS) {
        _toolbar->make_widget<window_traffic_lights_widget>();
    } else {
        tt_no_default();
    }

    _content = make_widget<grid_layout_widget>(_content_delegate);
}

[[nodiscard]] bool
window_widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (super::update_constraints(display_time_point, need_reconstrain)) {
        ttlet toolbar_size = _toolbar->preferred_size();
        ttlet content_size = _content->preferred_size();

        float min_width = std::max(toolbar_size.width().minimum(), content_size.width().minimum());
        float max_width = std::min({toolbar_size.width().maximum(), content_size.width().maximum(), window.virtual_screen_size().width()});

        float min_height = toolbar_size.height().minimum() + content_size.height().minimum();
        float max_height = std::min(toolbar_size.height().maximum() + content_size.height().maximum(), window.virtual_screen_size().height());

        _preferred_size = interval_extent2{extent2{min_width, min_height}, extent2{max_width, max_height}};
        return true;
    } else {
        return false;
    }
}

void window_widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
    if (need_layout) {
        ttlet toolbar_size = _toolbar->preferred_size();
        ttlet toolbar_height = toolbar_size.minimum().height();
        ttlet toolbar_rectangle = aarectangle{0.0f, rectangle().height() - toolbar_height, rectangle().width(), toolbar_height};
        _toolbar->set_layout_parameters_from_parent(toolbar_rectangle);

        ttlet content_size = _content->preferred_size();
        ttlet content_rectangle = aarectangle{0.0f, 0.0f, rectangle().width(), rectangle().height() - toolbar_height};
        _content->set_layout_parameters_from_parent(content_rectangle);
    }

    abstract_container_widget::update_layout(display_time_point, need_layout);
}

hit_box window_widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    constexpr float BORDER_WIDTH = 10.0f;

    ttlet is_on_left_edge = position.x() <= BORDER_WIDTH;
    ttlet is_on_right_edge = position.x() >= (_size.width() - BORDER_WIDTH);
    ttlet is_on_bottom_edge = position.y() <= BORDER_WIDTH;
    ttlet is_on_top_edge = position.y() >= (_size.height() - BORDER_WIDTH);

    ttlet is_on_bottom_left_corner = is_on_bottom_edge && is_on_left_edge;
    ttlet is_on_bottom_right_corner = is_on_bottom_edge && is_on_right_edge;
    ttlet is_on_top_left_corner = is_on_top_edge && is_on_left_edge;
    ttlet is_on_top_right_corner = is_on_top_edge && is_on_right_edge;

    auto r = hit_box{weak_from_this(), _draw_layer};
    if (is_on_bottom_left_corner) {
        return {weak_from_this(), _draw_layer, hit_box::Type::BottomLeftResizeCorner};
    } else if (is_on_bottom_right_corner) {
        return {weak_from_this(), _draw_layer, hit_box::Type::BottomRightResizeCorner};
    } else if (is_on_top_left_corner) {
        return {weak_from_this(), _draw_layer, hit_box::Type::TopLeftResizeCorner};
    } else if (is_on_top_right_corner) {
        return {weak_from_this(), _draw_layer, hit_box::Type::TopRightResizeCorner};
    } else if (is_on_left_edge) {
        r.type = hit_box::Type::LeftResizeBorder;
    } else if (is_on_right_edge) {
        r.type = hit_box::Type::RightResizeBorder;
    } else if (is_on_bottom_edge) {
        r.type = hit_box::Type::BottomResizeBorder;
    } else if (is_on_top_edge) {
        r.type = hit_box::Type::TopResizeBorder;
    }

    if ((is_on_left_edge && left_resize_border_has_priority) || (is_on_right_edge && right_resize_border_has_priority) ||
        (is_on_bottom_edge && bottom_resize_border_has_priority) || (is_on_top_edge && top_resize_border_has_priority)) {
        return r;
    }

    for (ttlet &child : _children) {
        r = std::max(r, child->hitbox_test(point2{child->parent_to_local() * position}));
    }

    return r;
}

} // namespace tt
