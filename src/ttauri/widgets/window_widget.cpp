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
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"
#include "../scoped_buffer.hpp"

namespace tt::inline v1 {

void window_widget::constructor_implementation() noexcept
{
    _toolbar = std::make_unique<toolbar_widget>(window, this);

    if (theme().operating_system == operating_system::windows) {
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
        _system_menu = &_toolbar->make_widget<system_menu_widget>();
        _title_callback = title.subscribe([this] {
            window.gui.run([this] {
                this->_system_menu->icon = this->title->icon;
            });
        });
#endif
        _toolbar->make_widget<window_traffic_lights_widget, horizontal_alignment::right>();
    } else if (theme().operating_system == operating_system::macos) {
        _toolbar->make_widget<window_traffic_lights_widget>();
    } else {
        tt_no_default();
    }

    _content = std::make_unique<grid_widget>(window, this, _content_delegate);
}

[[nodiscard]] pmr::generator<widget *> window_widget::children(std::pmr::polymorphic_allocator<> &) const noexcept
{
    co_yield _toolbar.get();
    co_yield _content.get();
}

widget_constraints const &window_widget::set_constraints() noexcept
{
    _layout = {};
    ttlet toolbar_constraints = _toolbar->set_constraints();
    ttlet content_constraints = _content->set_constraints();

    auto minimum_width = std::max(
        toolbar_constraints.margins.left() + toolbar_constraints.minimum.width() + toolbar_constraints.margins.right(),
        content_constraints.margins.left() + content_constraints.minimum.width() + content_constraints.margins.right());
    auto preferred_width = std::max(
        toolbar_constraints.margins.left() + toolbar_constraints.preferred.width() + toolbar_constraints.margins.right(),
        content_constraints.margins.left() + content_constraints.preferred.width() + content_constraints.margins.right());
    auto maximum_width = std::max(
        toolbar_constraints.margins.left() + toolbar_constraints.maximum.width() + toolbar_constraints.margins.right(),
        content_constraints.margins.left() + content_constraints.maximum.width() + content_constraints.margins.right());

    // clang-format off
    auto minimum_height =
        toolbar_constraints.margins.top() +
        toolbar_constraints.preferred.height() +
        std::max(toolbar_constraints.margins.bottom(), content_constraints.margins.top()) +
        content_constraints.minimum.height() +
        content_constraints.margins.bottom();
    auto preferred_height =
        toolbar_constraints.margins.top() +
        toolbar_constraints.preferred.height() +
        std::max(toolbar_constraints.margins.bottom(), content_constraints.margins.top()) +
        content_constraints.preferred.height() +
        content_constraints.margins.bottom();
    auto maximum_height =
        toolbar_constraints.margins.top() +
        toolbar_constraints.preferred.height() +
        std::max(toolbar_constraints.margins.bottom(), content_constraints.margins.top()) +
        content_constraints.maximum.height() +
        content_constraints.margins.bottom();
    // clang-format on

    // The operating system also has a minimum and maximum size, these sizes
    // are more important than the calculated sizes.
    ttlet minimum_window_size = os_settings::minimum_window_size();
    ttlet maximum_window_size = os_settings::maximum_window_size();

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
        ttlet toolbar_height = _toolbar->constraints().preferred.height();
        ttlet between_margin = std::max(_toolbar->constraints().margins.bottom(), _content->constraints().margins.top());

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
    _toolbar->set_layout(layout.transform(_toolbar_rectangle));
    _content->set_layout(layout.transform(_content_rectangle));
}

void window_widget::draw(draw_context const &context) noexcept
{
    if (visible) {
        _toolbar->draw(context);
        _content->draw(context);
    }
}

hitbox window_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    constexpr float BORDER_WIDTH = 10.0f;

    ttlet can_resize_w = _constraints.minimum.width() != _constraints.maximum.width();
    ttlet can_resize_h = _constraints.minimum.height() != _constraints.maximum.height();

    ttlet is_on_l_edge = position.x() <= BORDER_WIDTH;
    ttlet is_on_r_edge = position.x() >= (layout().width() - BORDER_WIDTH);
    ttlet is_on_b_edge = position.y() <= BORDER_WIDTH;
    ttlet is_on_t_edge = position.y() >= (layout().height() - BORDER_WIDTH);

    ttlet is_on_lb_corner = is_on_l_edge and is_on_b_edge;
    ttlet is_on_rb_corner = is_on_r_edge and is_on_b_edge;
    ttlet is_on_lt_corner = is_on_r_edge and is_on_t_edge;
    ttlet is_on_rt_corner = is_on_l_edge and is_on_t_edge;
    ttlet is_on_corner = is_on_lb_corner or is_on_rb_corner or is_on_lt_corner or is_on_rt_corner;

    ttlet is_on_l_resizer = can_resize_w and is_on_l_edge;
    ttlet is_on_r_resizer = can_resize_w and is_on_r_edge;
    ttlet is_on_b_resizer = can_resize_h and is_on_b_edge;
    ttlet is_on_t_resizer = can_resize_h and is_on_t_edge;

    ttlet is_on_lb_resizer = is_on_l_resizer and is_on_b_resizer;
    ttlet is_on_rb_resizer = is_on_r_resizer and is_on_b_resizer;
    ttlet is_on_lt_resizer = is_on_l_resizer and is_on_t_resizer;
    ttlet is_on_rt_resizer = is_on_r_resizer and is_on_t_resizer;

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
    tt_axiom(is_gui_thread());
    return theme().color(theme_color::fill, semantic_layer);
}

/** Defining on which edges the resize handle has priority over widget at a higher layer.
 */
void window_widget::set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
{
    tt_axiom(is_gui_thread());
    _left_resize_border_has_priority = left;
    _right_resize_border_has_priority = right;
    _bottom_resize_border_has_priority = bottom;
    _top_resize_border_has_priority = top;
}

[[nodiscard]] grid_widget &window_widget::content() noexcept
{
    tt_axiom(is_gui_thread());
    tt_axiom(_content);
    return *_content;
}

[[nodiscard]] toolbar_widget &window_widget::toolbar() noexcept
{
    tt_axiom(is_gui_thread());
    tt_axiom(_toolbar);
    return *_toolbar;
}

} // namespace tt::inline v1
