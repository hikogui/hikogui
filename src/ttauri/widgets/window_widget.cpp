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

namespace tt {

using namespace std;

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

void window_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    _toolbar->constrain();
    _content->constrain();

    _minimum_size = {
        std::max(_toolbar->minimum_size().width(), _content->minimum_size().width()),
        _toolbar->preferred_size().height() + _content->minimum_size().height()};

    _preferred_size = {
        std::max(_toolbar->preferred_size().width(), _content->preferred_size().width()),
        _toolbar->preferred_size().height() + _content->preferred_size().height()};

    _maximum_size = {_content->maximum_size().width(), _toolbar->preferred_size().height() + _content->maximum_size().height()};

    // Override maximum size and preferred size.
    _maximum_size = max(_maximum_size, _minimum_size);
    _preferred_size = clamp(_preferred_size, _minimum_size, _maximum_size);

    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void window_widget::set_layout(layout_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            ttlet toolbar_height = _toolbar->preferred_size().height();
            _toolbar_rectangle = aarectangle{0.0f, layout().height() - toolbar_height, layout().width(), toolbar_height};
            _content_rectangle = aarectangle{0.0f, 0.0f, layout().width(), layout().height() - toolbar_height};
        }
        _toolbar->set_layout(_toolbar_rectangle * context);
        _content->set_layout(_content_rectangle * context);
    }
}

void window_widget::draw(draw_context const &context) noexcept
{
    _toolbar->draw(context);
    _content->draw(context);
}

hitbox window_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    constexpr float BORDER_WIDTH = 10.0f;

    ttlet is_on_left_edge = position.x() <= BORDER_WIDTH;
    ttlet is_on_right_edge = position.x() >= (layout().width() - BORDER_WIDTH);
    ttlet is_on_bottom_edge = position.y() <= BORDER_WIDTH;
    ttlet is_on_top_edge = position.y() >= (layout().height() - BORDER_WIDTH);

    ttlet is_on_bottom_left_corner = is_on_bottom_edge && is_on_left_edge;
    ttlet is_on_bottom_right_corner = is_on_bottom_edge && is_on_right_edge;
    ttlet is_on_top_left_corner = is_on_top_edge && is_on_left_edge;
    ttlet is_on_top_right_corner = is_on_top_edge && is_on_right_edge;

    auto r = hitbox{this, position};
    if (is_on_bottom_left_corner) {
        return {this, position, hitbox::Type::BottomLeftResizeCorner};
    } else if (is_on_bottom_right_corner) {
        return {this, position, hitbox::Type::BottomRightResizeCorner};
    } else if (is_on_top_left_corner) {
        return {this, position, hitbox::Type::TopLeftResizeCorner};
    } else if (is_on_top_right_corner) {
        return {this, position, hitbox::Type::TopRightResizeCorner};
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

    auto buffer = pmr::scoped_buffer<256>{};
    for (auto *child : children(buffer.allocator())) {
        if (child) {
            r = std::max(r, child->hitbox_test(child->layout().from_parent * position));
        }
    }

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

} // namespace tt
