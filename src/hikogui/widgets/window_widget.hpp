// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/window_widget.hpp Defines window_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "toolbar_widget.hpp"
#include "system_menu_widget.hpp"
#include "grid_widget.hpp"
#include "window_traffic_lights_widget.hpp"
#include "../l10n/module.hpp"

namespace hi { inline namespace v1 {

/** The top-level window widget.
 * This widget is the top-level widget that is owned by the `gui_window`.
 * It contains as childs the toolbar and content `grid_widget`.
 *
 * @ingroup widgets
 */
class window_widget final : public widget {
public:
    using super = widget;

    observer<label> title;

    window_widget(forward_of<observer<label>> auto&& title) noexcept : super(nullptr), title(hi_forward(title))
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

    /** The background color of the window.
     * This function is used during rendering to use the optimized
     * GPU clear function.
     */
    [[nodiscard]] color background_color() noexcept
    {
        hi_axiom(loop::main().on_thread());
        return theme().color(semantic_color::fill, semantic_layer);
    }

    /** Get a reference to the window's content widget.
     * @see grid_widget
     * @return A reference to a grid_widget.
     */
    [[nodiscard]] grid_widget& content() noexcept
    {
        hi_axiom(loop::main().on_thread());
        hi_assert_not_null(_content);
        return *_content;
    }

    /** Get a reference to window's toolbar widget.
     *
     * @see toolbar_widget
     * @return A reference to a toolbar_widget.
     */
    [[nodiscard]] toolbar_widget& toolbar() noexcept
    {
        hi_axiom(loop::main().on_thread());
        hi_assert_not_null(_toolbar);
        return *_toolbar;
    }

    /// @privatesection
    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) noexcept override
    {
        co_yield *_toolbar;
        co_yield *_content;
    }
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        hi_assert_not_null(_content);
        hi_assert_not_null(_toolbar);

        _layout = {};
        _content_constraints = _content->update_constraints();
        _toolbar_constraints = _toolbar->update_constraints();

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
    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            hilet toolbar_height = _toolbar_constraints.preferred.height();
            hilet between_margin = std::max(_toolbar_constraints.margins.bottom(), _content_constraints.margins.top());

            hilet toolbar_rectangle = aarectangle{
                point2{
                    _toolbar_constraints.margins.left(), context.height() - toolbar_height - _toolbar_constraints.margins.top()},
                point2{
                    context.width() - _toolbar_constraints.margins.right(),
                    context.height() - _toolbar_constraints.margins.top()}};
            _toolbar_shape = box_shape{_toolbar_constraints, toolbar_rectangle, theme().baseline_adjustment()};

            hilet content_rectangle = aarectangle{
                point2{_content_constraints.margins.left(), _content_constraints.margins.bottom()},
                point2{context.width() - _content_constraints.margins.right(), toolbar_rectangle.bottom() - between_margin}};
            _content_shape = box_shape{_content_constraints, content_rectangle, theme().baseline_adjustment()};
        }
        _toolbar->set_layout(context.transform(_toolbar_shape));
        _content->set_layout(context.transform(_content_shape));
    }
    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            context.draw_box(_layout, _layout.rectangle(), background_color(), background_color());

            _toolbar->draw(context);
            _content->draw(context);
        }
    }
    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
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
    bool handle_event(gui_event const& event) noexcept override
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
    bool process_event(gui_event const& event) const noexcept override
    {
        if (_window) {
            return _window->process_event(event);
        } else {
            // Since there is no window, pretend that the message was handled.
            return true;
        }
    }
    void set_window(gui_window *window) noexcept override
    {
        _window = window;
        if (_window) {
            _window->set_title(*title);
        }
    }
    [[nodiscard]] gui_window *window() const noexcept override
    {
        return _window;
    }
    /// @endprivatesection
private:
    gui_window *_window = nullptr;

    std::unique_ptr<grid_widget> _content;
    box_constraints _content_constraints;
    box_shape _content_shape;

    std::unique_ptr<toolbar_widget> _toolbar;
    box_constraints _toolbar_constraints;
    box_shape _toolbar_shape;

    mutable bool _can_resize_width;
    mutable bool _can_resize_height;

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
    system_menu_widget *_system_menu = nullptr;
#endif
};

}} // namespace hi::v1
