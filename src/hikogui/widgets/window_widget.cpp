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
#include <memory>

namespace hi::inline v1 {

window_widget::window_widget(gui_window *window) noexcept : super(nullptr), _window(window)
{
    hi_assert_not_null(_window);
    _toolbar = std::make_unique<toolbar_widget>(this);
    _content = std::make_unique<grid_widget>(this);
}

[[nodiscard]] generator<widget *> window_widget::children() const noexcept
{
    co_yield _toolbar.get();
    co_yield _content.get();
}

widget_constraints const& window_widget::set_constraints(set_constraints_context const& context) noexcept
{
    _layout = {};
    _toolbar->set_constraints(context);
    _content->set_constraints(context);

    hi_axiom(_toolbar->constraints().margins.left() == 0.0f);
    hi_axiom(_toolbar->constraints().margins.right() == 0.0f);
    hi_axiom(_toolbar->constraints().margins.top() == 0.0f);
    hi_axiom(_toolbar->constraints().minimum.height() == _toolbar->constraints().preferred.height());
    hi_axiom(_toolbar->constraints().minimum.height() == _toolbar->constraints().maximum.height());

    hilet content_margin_width = _content->constraints().margins.left() + _content->constraints().margins.right();
    hilet between_margin = std::max(_toolbar->constraints().margins.bottom(), _content->constraints().margins.top());

    auto minimum_width =
        std::max(_toolbar->constraints().minimum.width(), content_margin_width + _content->constraints().minimum.width());
    auto preferred_width =
        std::max(_toolbar->constraints().preferred.width(), content_margin_width + _content->constraints().preferred.width());
    auto maximum_width =
        std::max(_toolbar->constraints().maximum.width(), content_margin_width + _content->constraints().maximum.width());

    // clang-format off
    auto minimum_height =
        _toolbar->constraints().minimum.height() +
        between_margin +
        _content->constraints().minimum.height() +
        _content->constraints().margins.bottom();
    auto preferred_height =
        _toolbar->constraints().minimum.height() +
        between_margin +
        _content->constraints().preferred.height() +
        _content->constraints().margins.bottom();
    auto maximum_height =
        _toolbar->constraints().minimum.height() +
        between_margin +
        _content->constraints().maximum.height() +
        _content->constraints().margins.bottom();
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

void window_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet toolbar_height = _toolbar->constraints().minimum.height();
        hilet between_margin = std::max(_toolbar->constraints().margins.bottom(), _content->constraints().margins.top());

        // The toolbar covers the top of the window, hugging all edges without margins.
        _toolbar_rectangle =
            aarectangle{point2{0.0f, context.height() - toolbar_height}, point2{context.width(), context.height()}};

        _content_rectangle = aarectangle{
            point2{_content->constraints().margins.left(), _content->constraints().margins.bottom()},
            point2{context.width() - _content->constraints().margins.right(), _toolbar_rectangle.bottom() - between_margin}};
    }
    _toolbar->set_layout(context.transform(_toolbar_rectangle, _toolbar->constraints().baseline));
    _content->set_layout(context.transform(_content_rectangle, _content->constraints().baseline));
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

    hilet is_on_l_edge = position.x() <= BORDER_WIDTH;
    hilet is_on_r_edge = position.x() >= (layout().width() - BORDER_WIDTH);
    hilet is_on_b_edge = position.y() <= BORDER_WIDTH;
    hilet is_on_t_edge = position.y() >= (layout().height() - BORDER_WIDTH);

    // Corner resize has always priority.
    if (is_on_l_edge and is_on_b_edge) {
        switch (resize_axis()) {
        case axis::both:
            return {this, position, hitbox_type::bottom_left_resize_corner};
        case axis::width:
            return {this, position, hitbox_type::left_resize_border};
        case axis::height:
            return {this, position, hitbox_type::bottom_resize_border};
        default:;
        }
    } else if (is_on_r_edge and is_on_b_edge) {
        switch (resize_axis()) {
        case axis::both:
            return {this, position, hitbox_type::bottom_right_resize_corner};
        case axis::width:
            return {this, position, hitbox_type::right_resize_border};
        case axis::height:
            return {this, position, hitbox_type::bottom_resize_border};
        default:;
        }
    } else if (is_on_l_edge and is_on_t_edge) {
        switch (resize_axis()) {
        case axis::both:
            return {this, position, hitbox_type::top_left_resize_corner};
        case axis::width:
            return {this, position, hitbox_type::left_resize_border};
        case axis::height:
            return {this, position, hitbox_type::top_resize_border};
        default:;
        }
    } else if (is_on_r_edge and is_on_t_edge) {
        switch (resize_axis()) {
        case axis::both:
            return {this, position, hitbox_type::top_right_resize_corner};
        case axis::width:
            return {this, position, hitbox_type::right_resize_border};
        case axis::height:
            return {this, position, hitbox_type::top_resize_border};
        default:;
        }
    }

    // Border resize only has priority if there is no scroll-bar in the way.
    if (r.type != hitbox_type::scroll_bar) {
        if (is_on_l_edge and to_bool(resize_axis() & axis::width)) {
            return {this, position, hitbox_type::left_resize_border};
        } else if (is_on_r_edge and to_bool(resize_axis() & axis::width)) {
            return {this, position, hitbox_type::right_resize_border};
        } else if (is_on_b_edge and to_bool(resize_axis() & axis::height)) {
            return {this, position, hitbox_type::bottom_resize_border};
        } else if (is_on_t_edge and to_bool(resize_axis() & axis::height)) {
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
