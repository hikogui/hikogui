// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "window_widget.hpp"
#include "window_traffic_lights_widget.hpp"
#include "toolbar_widget.hpp"
#include "grid_widget.hpp"
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "system_menu_widget.hpp"
#endif
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"
#include "../scoped_buffer.hpp"

namespace hi::inline v1 {

void window_widget::constructor_implementation() noexcept
{
    _toolbar = std::make_unique<toolbar_widget>(window, this);

    if (theme().operating_system == operating_system::windows) {
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
        _system_menu = &_toolbar->make_widget<system_menu_widget>();
        this->_system_menu->icon = this->title.get<"icon">();
#endif
        _toolbar->make_widget<window_traffic_lights_widget, horizontal_alignment::right>();
    } else if (theme().operating_system == operating_system::macos) {
        _toolbar->make_widget<window_traffic_lights_widget>();
    } else {
        hi_no_default();
    }

    _content = std::make_unique<grid_widget>(window, this);
}

[[nodiscard]] generator<widget *> window_widget::children() const noexcept
{
    co_yield _toolbar.get();
    co_yield _content.get();
}

widget_constraints const &window_widget::set_constraints() noexcept
{
    _layout = {};
    _toolbar_constraints = _toolbar->set_constraints();
    _content_constraints = _content->set_constraints();

    auto minimum_width = std::max(
        _toolbar_constraints.margins.left() + _toolbar_constraints.minimum.width() + _toolbar_constraints.margins.right(),
        _content_constraints.margins.left() + _content_constraints.minimum.width() + _content_constraints.margins.right());
    auto preferred_width = std::max(
        _toolbar_constraints.margins.left() + _toolbar_constraints.preferred.width() + _toolbar_constraints.margins.right(),
        _content_constraints.margins.left() + _content_constraints.preferred.width() + _content_constraints.margins.right());
    auto maximum_width = std::max(
        _toolbar_constraints.margins.left() + _toolbar_constraints.maximum.width() + _toolbar_constraints.margins.right(),
        _content_constraints.margins.left() + _content_constraints.maximum.width() + _content_constraints.margins.right());

    // clang-format off
    auto minimum_height =
        _toolbar_constraints.margins.top() +
        _toolbar_constraints.preferred.height() +
        std::max(_toolbar_constraints.margins.bottom(), _content_constraints.margins.top()) +
        _content_constraints.minimum.height() +
        _content_constraints.margins.bottom();
    auto preferred_height =
        _toolbar_constraints.margins.top() +
        _toolbar_constraints.preferred.height() +
        std::max(_toolbar_constraints.margins.bottom(), _content_constraints.margins.top()) +
        _content_constraints.preferred.height() +
        _content_constraints.margins.bottom();
    auto maximum_height =
        _toolbar_constraints.margins.top() +
        _toolbar_constraints.preferred.height() +
        std::max(_toolbar_constraints.margins.bottom(), _content_constraints.margins.top()) +
        _content_constraints.maximum.height() +
        _content_constraints.margins.bottom();
    // clang-format on

    // The operating system also has a minimum and maximum size, these sizes
    // are more important than the calculated sizes.
    hilet minimum_window_size = os_settings::minimum_window_size();
    hilet maximum_window_size = os_settings::maximum_window_size();

    inplace_max(minimum_width, minimum_window_size.width());
    inplace_max(minimum_height, minimum_window_size.height());

    inplace_clamp(maximum_width, minimum_width, maximum_window_size.width());
    inplace_clamp(maximum_height, minimum_height, maximum_window_size.height());

    inplace_clamp(preferred_width, minimum_width, maximum_width);
    inplace_clamp(preferred_height, minimum_height, maximum_height);

    return _constraints = {{minimum_width, minimum_height}, {preferred_width, preferred_height}, {maximum_width, maximum_height}};
}

void window_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        hilet toolbar_height = _toolbar->constraints().preferred.height();
        hilet between_margin = std::max(_toolbar->constraints().margins.bottom(), _content->constraints().margins.top());

        _toolbar_rectangle = aarectangle{
            point2{
                _toolbar->constraints().margins.left(), layout.height() - toolbar_height - _toolbar->constraints().margins.top()},
            point2{
                layout.width() - _toolbar->constraints().margins.right(),
                layout.height() - _toolbar->constraints().margins.top()}};

        _content_rectangle = aarectangle{
            point2{_content->constraints().margins.left(), _content->constraints().margins.bottom()},
            point2{layout.width() - _content->constraints().margins.right(), _toolbar_rectangle.bottom() - between_margin}};
    }
    _toolbar->set_layout(layout.transform(_toolbar_rectangle, _toolbar_constraints.baseline));
    _content->set_layout(layout.transform(_content_rectangle, _content_constraints.baseline));
}

void window_widget::draw(draw_context const &context) noexcept
{
    if (*mode > widget_mode::invisible) {
        _toolbar->draw(context);
        _content->draw(context);
    }
}

hitbox window_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(is_gui_thread());

    constexpr float BORDER_WIDTH = 10.0f;

    hilet can_resize_w = _constraints.minimum.width() != _constraints.maximum.width();
    hilet can_resize_h = _constraints.minimum.height() != _constraints.maximum.height();

    hilet is_on_l_edge = position.x() <= BORDER_WIDTH;
    hilet is_on_r_edge = position.x() >= (layout().width() - BORDER_WIDTH);
    hilet is_on_b_edge = position.y() <= BORDER_WIDTH;
    hilet is_on_t_edge = position.y() >= (layout().height() - BORDER_WIDTH);

    hilet is_on_lb_corner = is_on_l_edge and is_on_b_edge;
    hilet is_on_rb_corner = is_on_r_edge and is_on_b_edge;
    hilet is_on_lt_corner = is_on_r_edge and is_on_t_edge;
    hilet is_on_rt_corner = is_on_l_edge and is_on_t_edge;
    hilet is_on_corner = is_on_lb_corner or is_on_rb_corner or is_on_lt_corner or is_on_rt_corner;

    hilet is_on_l_resizer = can_resize_w and is_on_l_edge;
    hilet is_on_r_resizer = can_resize_w and is_on_r_edge;
    hilet is_on_b_resizer = can_resize_h and is_on_b_edge;
    hilet is_on_t_resizer = can_resize_h and is_on_t_edge;

    hilet is_on_lb_resizer = is_on_l_resizer and is_on_b_resizer;
    hilet is_on_rb_resizer = is_on_r_resizer and is_on_b_resizer;
    hilet is_on_lt_resizer = is_on_l_resizer and is_on_t_resizer;
    hilet is_on_rt_resizer = is_on_r_resizer and is_on_t_resizer;

    auto r = hitbox{this, position};
    if (is_on_lb_resizer) {
        r.type = hitbox::Type::BottomLeftResizeCorner;
    } else if (is_on_rb_resizer) {
        r.type = hitbox::Type::BottomRightResizeCorner;
    } else if (is_on_lt_resizer) {
        r.type = hitbox::Type::TopLeftResizeCorner;
    } else if (is_on_rt_resizer) {
        r.type = hitbox::Type::TopRightResizeCorner;
    } else if (is_on_l_resizer) {
        r.type = hitbox::Type::LeftResizeBorder;
    } else if (is_on_r_resizer) {
        r.type = hitbox::Type::RightResizeBorder;
    } else if (is_on_b_resizer) {
        r.type = hitbox::Type::BottomResizeBorder;
    } else if (is_on_t_resizer) {
        r.type = hitbox::Type::TopResizeBorder;
    }

    if (is_on_corner and r.type != hitbox::Type::Default) {
        // Resizers on corners always have priority.
        return r;
    }

    if ((is_on_l_resizer and _left_resize_border_has_priority) or (is_on_r_resizer and _right_resize_border_has_priority) or
        (is_on_b_resizer and _bottom_resize_border_has_priority) or (is_on_t_resizer and _top_resize_border_has_priority)) {
        // Resizers on edges only have priority if there is not a scroll bar on that edge.
        return r;
    }

    // Otherwise children have priority.
    r = _toolbar->hitbox_test_from_parent(position, r);
    r = _content->hitbox_test_from_parent(position, r);
    return r;
}

[[nodiscard]] color window_widget::background_color() noexcept
{
    hi_axiom(is_gui_thread());
    return theme().color(semantic_color::fill, semantic_layer);
}

/** Defining on which edges the resize handle has priority over widget at a higher layer.
 */
void window_widget::set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
{
    hi_axiom(is_gui_thread());
    _left_resize_border_has_priority = left;
    _right_resize_border_has_priority = right;
    _bottom_resize_border_has_priority = bottom;
    _top_resize_border_has_priority = top;
}

[[nodiscard]] grid_widget &window_widget::content() noexcept
{
    hi_axiom(is_gui_thread());
    hi_assert_not_null(_content);
    return *_content;
}

[[nodiscard]] toolbar_widget &window_widget::toolbar() noexcept
{
    hi_axiom(is_gui_thread());
    hi_assert_not_null(_toolbar);
    return *_toolbar;
}

bool window_widget::handle_event(gui_event const& event) noexcept
{
    switch (event.type()) {
    case gui_event_type::gui_toolbar_open:
        window.update_keyboard_target(this, keyboard_focus_group::toolbar, keyboard_focus_direction::forward);
        return true;

    case gui_event_type::gui_sysmenu_open:
        if (*mode >= widget_mode::partial) {
            window.open_system_menu();
            return true;
        }
        break;
    }
    return super::handle_event(event);
}

} // namespace hi::inline v1
