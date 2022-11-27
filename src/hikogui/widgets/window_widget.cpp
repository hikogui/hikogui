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
#include "../scoped_buffer.hpp"

namespace hi::inline v1 {

void window_widget::constructor_implementation() noexcept
{
    _toolbar = std::make_unique<toolbar_widget>(this);

    if (operating_system::current == operating_system::windows) {
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
        _system_menu = &_toolbar->make_widget<system_menu_widget>();
        this->_system_menu->icon = this->title.get<"icon">();
#endif
        _toolbar->make_widget<window_traffic_lights_widget, horizontal_alignment::right>();
    } else if (operating_system::current == operating_system::macos) {
        _toolbar->make_widget<window_traffic_lights_widget>();
    } else {
        hi_no_default();
    }

    _content = std::make_unique<grid_widget>(this);
}

[[nodiscard]] generator<widget *> window_widget::children() const noexcept
{
    co_yield _toolbar.get();
    co_yield _content.get();
}

box_constraints const& window_widget::set_constraints(set_constraints_context const& context) noexcept
{
    _layout = {};
    _toolbar_constraints = _toolbar->set_constraints(context);
    _content_constraints = _content->set_constraints(context);

    _constraints = {};
    _constraints.minimum_width = std::max(
        _toolbar_constraints.margin_left + _toolbar_constraints.minimum_width + _toolbar_constraints.margin_right,
        _content_constraints.margin_left + _content_constraints.minimum_width + _content_constraints.margin_right);
    _constraints.preferred_width = std::max(
        _toolbar_constraints.margin_left + _toolbar_constraints.preferred_width + _toolbar_constraints.margin_right,
        _content_constraints.margin_left + _content_constraints.preferred_width + _content_constraints.margin_right);
    _constraints.maximum_width = std::max(
        _toolbar_constraints.margin_left + _toolbar_constraints.maximum_width + _toolbar_constraints.margin_right,
        _content_constraints.margin_left + _content_constraints.maximum_width + _content_constraints.margin_right);

    // clang-format off
    _constraints.minimum_height =
        _toolbar_constraints.margin_top +
        _toolbar_constraints.preferred_height +
        std::max(_toolbar_constraints.margin_bottom, _content_constraints.margin_top) +
        _content_constraints.minimum_height +
        _content_constraints.margin_bottom;
    _constraints.preferred_height =
        _toolbar_constraints.margin_top +
        _toolbar_constraints.preferred_height +
        std::max(_toolbar_constraints.margin_bottom, _content_constraints.margin_top) +
        _content_constraints.preferred_height +
        _content_constraints.margin_bottom;
    _constraints.maximum_height =
        _toolbar_constraints.margin_top +
        _toolbar_constraints.preferred_height +
        std::max(_toolbar_constraints.margin_bottom, _content_constraints.margin_top) +
        _content_constraints.maximum_height +
        _content_constraints.margin_bottom;
    // clang-format on

    // The operating system also has a minimum and maximum size, these sizes
    // are more important than the calculated sizes.
    inplace_max(_constraints.minimum_width, os_settings::minimum_window_width());
    inplace_max(_constraints.minimum_height, os_settings::minimum_window_height());

    inplace_clamp(_constraints.maximum_width, _constraints.minimum_width, os_settings::maximum_window_width());
    inplace_clamp(_constraints.maximum_height, _constraints.minimum_height, os_settings::maximum_window_height());

    inplace_clamp(_constraints.preferred_width, _constraints.minimum_width, _constraints.maximum_width);
    inplace_clamp(_constraints.preferred_height, _constraints.minimum_height, _constraints.maximum_height);

    return _constraints;
}

void window_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet toolbar_height = narrow_cast<float>(_toolbar_constraints.preferred_height);
        hilet between_margin =
            std::max(narrow_cast<float>(_toolbar_constraints.margin_bottom), narrow_cast<float>(_content_constraints.margin_top));

        hilet toolbar_rectangle = aarectangle{
            point2{
                narrow_cast<float>(_toolbar_constraints.margin_left),
                narrow_cast<float>(context.height()) - toolbar_height - narrow_cast<float>(_toolbar_constraints.margin_top)},
            point2{
                narrow_cast<float>(context.width()) - narrow_cast<float>(_toolbar_constraints.margin_right),
                narrow_cast<float>(context.height()) - narrow_cast<float>(_toolbar_constraints.margin_top)}};
        _toolbar_shape = box_shape{_toolbar_constraints, toolbar_rectangle, context.theme->baseline_adjustment};

        hilet content_rectangle = aarectangle{
            point2{narrow_cast<float>(_content_constraints.margin_left), narrow_cast<float>(_content_constraints.margin_bottom)},
            point2{
                narrow_cast<float>(context.width()) - narrow_cast<float>(_content_constraints.margin_right),
                toolbar_rectangle.bottom() - between_margin}};
        _content_shape = box_shape{_content_constraints, content_rectangle, context.theme->baseline_adjustment};
    }
    _toolbar->set_layout(context.transform(_toolbar_shape));
    _content->set_layout(context.transform(_content_shape));
}

void window_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible) {
        _toolbar->draw(context);
        _content->draw(context);
    }
}

hitbox window_widget::hitbox_test(point3 position) const noexcept
{
    constexpr float BORDER_WIDTH = 10.0f;

    hi_axiom(loop::main().on_thread());

    auto r = _toolbar->hitbox_test_from_parent(position);
    r = _content->hitbox_test_from_parent(position, r);

    hilet can_resize_w = _constraints.minimum_width != _constraints.maximum_width;
    hilet can_resize_h = _constraints.minimum_height != _constraints.maximum_height;

    hilet is_on_l_edge = position.x() <= BORDER_WIDTH;
    hilet is_on_r_edge = position.x() >= (layout().width() - BORDER_WIDTH);
    hilet is_on_b_edge = position.y() <= BORDER_WIDTH;
    hilet is_on_t_edge = position.y() >= (layout().height() - BORDER_WIDTH);

    // Corner resize has always priority.
    if (is_on_l_edge and is_on_b_edge) {
        if (can_resize_w and can_resize_h) {
            return {this, position, hitbox_type::bottom_left_resize_corner};
        } else if (can_resize_w) {
            return {this, position, hitbox_type::left_resize_border};
        } else if (can_resize_h) {
            return {this, position, hitbox_type::bottom_resize_border};
        }
    } else if (is_on_r_edge and is_on_b_edge) {
        if (can_resize_w and can_resize_h) {
            return {this, position, hitbox_type::bottom_right_resize_corner};
        } else if (can_resize_w) {
            return {this, position, hitbox_type::right_resize_border};
        } else if (can_resize_h) {
            return {this, position, hitbox_type::bottom_resize_border};
        }
    } else if (is_on_l_edge and is_on_t_edge) {
        if (can_resize_w and can_resize_h) {
            return {this, position, hitbox_type::top_left_resize_corner};
        } else if (can_resize_w) {
            return {this, position, hitbox_type::left_resize_border};
        } else if (can_resize_h) {
            return {this, position, hitbox_type::top_resize_border};
        }
    } else if (is_on_r_edge and is_on_t_edge) {
        if (can_resize_w and can_resize_h) {
            return {this, position, hitbox_type::top_right_resize_corner};
        } else if (can_resize_w) {
            return {this, position, hitbox_type::right_resize_border};
        } else if (can_resize_h) {
            return {this, position, hitbox_type::top_resize_border};
        }
    }

    // Border resize only has priority if there is no scroll-bar in the way.
    if (r.type != hitbox_type::scroll_bar) {
        if (is_on_l_edge and can_resize_w) {
            return {this, position, hitbox_type::left_resize_border};
        } else if (is_on_r_edge and can_resize_w) {
            return {this, position, hitbox_type::right_resize_border};
        } else if (is_on_b_edge and can_resize_h) {
            return {this, position, hitbox_type::bottom_resize_border};
        } else if (is_on_t_edge and can_resize_h) {
            return {this, position, hitbox_type::top_resize_border};
        }
    }

    return r;
}

[[nodiscard]] color window_widget::background_color() noexcept
{
    hi_axiom(loop::main().on_thread());
    return layout().theme->color(semantic_color::fill, semantic_layer);
}

[[nodiscard]] grid_widget& window_widget::content() noexcept
{
    hi_axiom(loop::main().on_thread());
    hi_assert_not_null(_content);
    return *_content;
}

[[nodiscard]] toolbar_widget& window_widget::toolbar() noexcept
{
    hi_axiom(loop::main().on_thread());
    hi_assert_not_null(_toolbar);
    return *_toolbar;
}

bool window_widget::handle_event(gui_event const& event) noexcept
{
    using enum gui_event_type;

    switch (event.type()) {
    case gui_toolbar_open:
        process_event(
            gui_event::window_set_keyboard_target(this, keyboard_focus_group::toolbar, keyboard_focus_direction::forward));
        return true;
    }
    return super::handle_event(event);
}

bool window_widget::process_event(gui_event const& event) const noexcept
{
    hi_assert_not_null(_window);
    return _window->process_event(event);
}

} // namespace hi::inline v1
