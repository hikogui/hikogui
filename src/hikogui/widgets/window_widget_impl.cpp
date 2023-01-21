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

[[nodiscard]] gui_window *window_widget::window() const noexcept
{
    return _window;
}

[[nodiscard]] hi::theme const& window_widget::theme() const noexcept
{
    hi_assert_not_null(_window);
    return _window->theme;
}

[[nodiscard]] gfx_surface const *window_widget::surface() const noexcept
{
    hi_assert_not_null(_window);
    return _window->surface.get();
}

[[nodiscard]] generator<widget const&> window_widget::children(bool include_invisible) const noexcept
{
    co_yield *_toolbar;
    co_yield *_content;
}

[[nodiscard]] box_constraints window_widget::update_constraints() noexcept
{
    hi_assert_not_null(_content);
    hi_assert_not_null(_toolbar);

    _layout = {};
    _content_constraints =_content->update_constraints();
    _toolbar_constraints =  _toolbar->update_constraints();

    auto r = box_constraints{};
    r.minimum.width() = std::max(
        _toolbar_constraints.margins.left() + _toolbar_constraints.minimum.width() + _toolbar_constraints.margins.right(),
        _content_constraints.margins.left() + _content_constraints.minimum.width() + _content_constraints.margins.right());
    r.preferred.width() = std::max(
        _toolbar_constraints.margins.left() + _toolbar_constraints.preferred.width() + _toolbar_constraints.margins.right(),
        _content_constraints.margins.left() + _content_constraints.preferred.width() + _content_constraints.margins.right());
    r.maximum.width() = std::min(
        _toolbar_constraints.margins.left() + _toolbar_constraints.maximum.width() + _toolbar_constraints.margins.right(),
        _content_constraints.margins.left() + _content_constraints.maximum.width() + _content_constraints.margins.right());

    // clang-format off
    r.minimum.height() =
        _toolbar_constraints.margins.top() +
        _toolbar_constraints.preferred.height() +
        std::max(_toolbar_constraints.margins.bottom(), _content_constraints.margins.top()) +
        _content_constraints.minimum.height() +
        _content_constraints.margins.bottom();
    r.preferred.height() =
        _toolbar_constraints.margins.top() +
        _toolbar_constraints.preferred.height() +
        std::max(_toolbar_constraints.margins.bottom(), _content_constraints.margins.top()) +
        _content_constraints.preferred.height() +
        _content_constraints.margins.bottom();
    r.maximum.height() =
        _toolbar_constraints.margins.top() +
        _toolbar_constraints.preferred.height() +
        std::max(_toolbar_constraints.margins.bottom(), _content_constraints.margins.top()) +
        _content_constraints.maximum.height() +
        _content_constraints.margins.bottom();
    // clang-format on

    // The operating system also has a minimum and maximum size, these sizes
    // are more important than the calculated sizes.
    inplace_max(r.minimum.width(), os_settings::minimum_window_width());
    inplace_max(r.minimum.height(), os_settings::minimum_window_height());

    inplace_clamp(r.maximum.width(), r.minimum.width(), os_settings::maximum_window_width());
    inplace_clamp(r.maximum.height(), r.minimum.height(), os_settings::maximum_window_height());

    inplace_clamp(r.preferred.width(), r.minimum.width(), r.maximum.width());
    inplace_clamp(r.preferred.height(), r.minimum.height(), r.maximum.height());

    _can_resize_width = r.minimum.width() != r.maximum.width();
    _can_resize_height = r.minimum.height() != r.maximum.height();

    return r;
}

void window_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet toolbar_height = _toolbar_constraints.preferred.height();
        hilet between_margin = std::max(_toolbar_constraints.margins.bottom(), _content_constraints.margins.top());

        hilet toolbar_rectangle = aarectanglei{
            point2i{
                _toolbar_constraints.margins.left(),
                context.height() - toolbar_height - _toolbar_constraints.margins.top()},
            point2i{
                context.width() - _toolbar_constraints.margins.right(),
                context.height() - _toolbar_constraints.margins.top()}};
        _toolbar_shape = box_shape{_toolbar_constraints, toolbar_rectangle, theme().baseline_adjustment()};

        hilet content_rectangle = aarectanglei{
            point2i{_content_constraints.margins.left(), _content_constraints.margins.bottom()},
            point2i{context.width() - _content_constraints.margins.right(), toolbar_rectangle.bottom() - between_margin}};
        _content_shape = box_shape{_content_constraints, content_rectangle, theme().baseline_adjustment()};
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

hitbox window_widget::hitbox_test(point2i position) const noexcept
{
    constexpr float BORDER_WIDTH = 10.0f;

    hi_axiom(loop::main().on_thread());

    auto r = _toolbar->hitbox_test_from_parent(position);
    r = _content->hitbox_test_from_parent(position, r);

    hilet is_on_l_edge = position.x() <= BORDER_WIDTH;
    hilet is_on_r_edge = position.x() >= (layout().width() - BORDER_WIDTH);
    hilet is_on_b_edge = position.y() <= BORDER_WIDTH;
    hilet is_on_t_edge = position.y() >= (layout().height() - BORDER_WIDTH);

    // Corner resize has always priority.
    if (is_on_l_edge and is_on_b_edge) {
        if (_can_resize_width and _can_resize_height) {
            return {id, _layout.elevation, hitbox_type::bottom_left_resize_corner};
        } else if (_can_resize_width) {
            return {id, _layout.elevation, hitbox_type::left_resize_border};
        } else if (_can_resize_height) {
            return {id, _layout.elevation, hitbox_type::bottom_resize_border};
        }
    } else if (is_on_r_edge and is_on_b_edge) {
        if (_can_resize_width and _can_resize_height) {
            return {id, _layout.elevation, hitbox_type::bottom_right_resize_corner};
        } else if (_can_resize_width) {
            return {id, _layout.elevation, hitbox_type::right_resize_border};
        } else if (_can_resize_height) {
            return {id, _layout.elevation, hitbox_type::bottom_resize_border};
        }
    } else if (is_on_l_edge and is_on_t_edge) {
        if (_can_resize_width and _can_resize_height) {
            return {id, _layout.elevation, hitbox_type::top_left_resize_corner};
        } else if (_can_resize_width) {
            return {id, _layout.elevation, hitbox_type::left_resize_border};
        } else if (_can_resize_height) {
            return {id, _layout.elevation, hitbox_type::top_resize_border};
        }
    } else if (is_on_r_edge and is_on_t_edge) {
        if (_can_resize_width and _can_resize_height) {
            return {id, _layout.elevation, hitbox_type::top_right_resize_corner};
        } else if (_can_resize_width) {
            return {id, _layout.elevation, hitbox_type::right_resize_border};
        } else if (_can_resize_height) {
            return {id, _layout.elevation, hitbox_type::top_resize_border};
        }
    }

    // Border resize only has priority if there is no scroll-bar in the way.
    if (r.type != hitbox_type::scroll_bar) {
        if (is_on_l_edge and _can_resize_width) {
            return {id, _layout.elevation, hitbox_type::left_resize_border};
        } else if (is_on_r_edge and _can_resize_width) {
            return {id, _layout.elevation, hitbox_type::right_resize_border};
        } else if (is_on_b_edge and _can_resize_height) {
            return {id, _layout.elevation, hitbox_type::bottom_resize_border};
        } else if (is_on_t_edge and _can_resize_height) {
            return {id, _layout.elevation, hitbox_type::top_resize_border};
        }
    }

    return r;
}

[[nodiscard]] color window_widget::background_color() noexcept
{
    hi_axiom(loop::main().on_thread());
    return theme().color(semantic_color::fill, semantic_layer);
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
            gui_event::window_set_keyboard_target(id, keyboard_focus_group::toolbar, keyboard_focus_direction::forward));
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
