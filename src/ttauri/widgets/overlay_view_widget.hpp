// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "grid_layout_widget.hpp"

namespace tt {

class overlay_view_widget final : public widget {
public:
    using super = widget;

    overlay_view_widget(gui_window &window, std::shared_ptr<widget> parent) noexcept : super(window, parent)
    {
        tt_axiom(is_gui_thread());

        if (parent) {
            // The overlay-widget will reset the semantic_layer as it is the bottom
            // layer of this virtual-window. However the draw-layer should be above
            // any other widget drawn.
            _draw_layer = parent->draw_layer() + 20.0f;
            _semantic_layer = 0;
            _margin = theme::global().margin;
        }
    }

    ~overlay_view_widget() {}

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());

        auto has_updated_contraints = super::update_constraints(display_time_point, need_reconstrain);

        if (has_updated_contraints) {
            tt_axiom(_content);
            _minimum_size = _content->minimum_size();
            _preferred_size = _content->preferred_size();
            _maximum_size = _content->maximum_size();
            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        }

        return has_updated_contraints;
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(is_gui_thread());

        need_layout |= _request_relayout.exchange(false);
        if (need_layout) {
            tt_axiom(_content);
            _content->set_layout_parameters_from_parent(rectangle(), rectangle(), 1.0f);
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (overlaps(context, _clipping_rectangle)) {
            draw_background(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    /** Make an overlay rectangle.
     * @param requested_rectangle A rectangle in the parent's local coordinate system.
     * @return A rectangle that fits the window's constraints in the parent's local coordinate system.
     */
    [[nodiscard]] aarectangle make_overlay_rectangle_from_parent(aarectangle requested_rectangle) const noexcept
    {
        tt_axiom(is_gui_thread());

        if (auto parent = _parent.lock()) {
            ttlet requested_window_rectangle = aarectangle{parent->local_to_window() * requested_rectangle};
            ttlet window_bounds = shrink(aarectangle{window.size}, _margin);
            ttlet response_window_rectangle = fit(window_bounds, requested_window_rectangle);
            return aarectangle{parent->window_to_local() * response_window_rectangle};
        } else {
            tt_no_default();
        }
    }

    template<typename WidgetType = grid_layout_widget, typename... Args>
    std::shared_ptr<WidgetType> make_widget(Args &&... args) noexcept
    {
        tt_axiom(is_gui_thread());

        auto widget = super::make_widget<WidgetType>(std::forward<Args>(args)...);
        tt_axiom(!_content);
        _content = widget;
        return widget;
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        return theme::global(theme_color::fill, _semantic_layer + 1);
    }

    [[nodiscard]] color foreground_color() const noexcept override
    {
        return theme::global(theme_color::border, _semantic_layer + 1);
    }

    void scroll_to_show(tt::rectangle rectangle) noexcept override
    {
        // An overlay is in an absolute position on the window,
        // so do not forward the scroll_to_show message to its parent.
    }

private:
    std::shared_ptr<widget> _content;

    void draw_background(draw_context context) noexcept
    {
        context.draw_box_with_border_outside(rectangle(), background_color(), foreground_color());
    }
};

} // namespace tt