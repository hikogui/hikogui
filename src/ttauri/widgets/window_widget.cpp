// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "window_widget.hpp"
#include "window_traffic_lights_widget.hpp"
#include "toolbar_widget.hpp"
#include "grid_widget.hpp"
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "system_menu_widget.hpp"
#endif

namespace tt {

using namespace std;

window_widget::~window_widget() {}

void window_widget::init() noexcept
{
    _toolbar = &make_widget<toolbar_widget>();

    if (theme::global().operating_system == operating_system::windows) {
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
        _system_menu = &_toolbar->make_widget<system_menu_widget>();
        _title_callback = title.subscribe([this]{
            run_on_gui_thread([this]{
                this->_system_menu->icon = (*this->title).icon;
            });
        });
#endif
        _toolbar->make_widget<window_traffic_lights_widget, horizontal_alignment::right>();
    } else if (theme::global().operating_system == operating_system::macos) {
        _toolbar->make_widget<window_traffic_lights_widget>();
    } else {
        tt_no_default();
    }

    _content = &make_widget<grid_widget>(_content_delegate);
}

[[nodiscard]] bool
window_widget::constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        _minimum_size = {
            std::max(_toolbar->minimum_size().width(), _content->minimum_size().width()),
            _toolbar->preferred_size().height() + _content->minimum_size().height()};

        _preferred_size = {
            std::max(_toolbar->preferred_size().width(), _content->preferred_size().width()),
            _toolbar->preferred_size().height() + _content->preferred_size().height()};

        _maximum_size = {
            _content->maximum_size().width(),
            _toolbar->preferred_size().height() + _content->maximum_size().height()};

        // Override maximum size and preferred size.
        _maximum_size = max(_maximum_size, _minimum_size);
        _preferred_size = clamp(_preferred_size, _minimum_size, _maximum_size);

        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

void window_widget::layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        ttlet toolbar_height = _toolbar->preferred_size().height();
        ttlet toolbar_rectangle = aarectangle{0.0f, rectangle().height() - toolbar_height, rectangle().width(), toolbar_height};
        _toolbar->set_layout_parameters_from_parent(toolbar_rectangle);

        ttlet content_rectangle = aarectangle{0.0f, 0.0f, rectangle().width(), rectangle().height() - toolbar_height};
        _content->set_layout_parameters_from_parent(content_rectangle);
    }

    super::layout(display_time_point, need_layout);
}

hitbox window_widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(is_gui_thread());

    constexpr float BORDER_WIDTH = 10.0f;

    ttlet is_on_left_edge = position.x() <= BORDER_WIDTH;
    ttlet is_on_right_edge = position.x() >= (_size.width() - BORDER_WIDTH);
    ttlet is_on_bottom_edge = position.y() <= BORDER_WIDTH;
    ttlet is_on_top_edge = position.y() >= (_size.height() - BORDER_WIDTH);

    ttlet is_on_bottom_left_corner = is_on_bottom_edge && is_on_left_edge;
    ttlet is_on_bottom_right_corner = is_on_bottom_edge && is_on_right_edge;
    ttlet is_on_top_left_corner = is_on_top_edge && is_on_left_edge;
    ttlet is_on_top_right_corner = is_on_top_edge && is_on_right_edge;

    auto r = hitbox{this, draw_layer};
    if (is_on_bottom_left_corner) {
        return {this, draw_layer, hitbox::Type::BottomLeftResizeCorner};
    } else if (is_on_bottom_right_corner) {
        return {this, draw_layer, hitbox::Type::BottomRightResizeCorner};
    } else if (is_on_top_left_corner) {
        return {this, draw_layer, hitbox::Type::TopLeftResizeCorner};
    } else if (is_on_top_right_corner) {
        return {this, draw_layer, hitbox::Type::TopRightResizeCorner};
    } else if (is_on_left_edge) {
        r.type = hitbox::Type::LeftResizeBorder;
    } else if (is_on_right_edge) {
        r.type = hitbox::Type::RightResizeBorder;
    } else if (is_on_bottom_edge) {
        r.type = hitbox::Type::BottomResizeBorder;
    } else if (is_on_top_edge) {
        r.type = hitbox::Type::TopResizeBorder;
    }

    if ((is_on_left_edge && _left_resize_border_has_priority) || (is_on_right_edge && _right_resize_border_has_priority) ||
        (is_on_bottom_edge && _bottom_resize_border_has_priority) || (is_on_top_edge && _top_resize_border_has_priority)) {
        return r;
    }

    for (ttlet &child : _children) {
        r = std::max(r, child->hitbox_test(point2{child->parent_to_local() * position}));
    }

    return r;
}

} // namespace tt
