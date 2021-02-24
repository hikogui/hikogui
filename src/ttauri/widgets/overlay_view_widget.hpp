// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "grid_layout_widget.hpp"

namespace tt {

class overlay_view_widget final : public abstract_container_widget {
public:
    using super = abstract_container_widget;

    overlay_view_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent) noexcept : super(window, parent)
    {
        if (parent) {
            // The overlay-widget will reset the semantic_layer as it is the bottom
            // layer of this virtual-window. However the draw-layer should be above
            // any other widget drawn.
            ttlet lock = std::scoped_lock(gui_system_mutex);
            _draw_layer = parent->draw_layer() + 20.0f;
            _semantic_layer = 0;
        }
    }

    ~overlay_view_widget() {}

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto has_updated_contraints = super::update_constraints(display_time_point, need_reconstrain);

        if (has_updated_contraints) {
            tt_axiom(_content);
            _preferred_size = _content->preferred_size();
            _preferred_base_line = _content->preferred_base_line();
        }

        return has_updated_contraints;
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            // The _window_rectangle, is not allowed to be beyond the edges of the actual window.
            // Change _window_rectangle to fit the window.
            ttlet window_rectangle_and_margin = expand(_window_rectangle, _margin);
            ttlet new_window_rectangle_and_margin = fit(aarect{f32x4{window.extent}}, window_rectangle_and_margin);
            _window_rectangle = shrink(new_window_rectangle_and_margin, _margin);
            _window_clipping_rectangle = _window_rectangle;

            tt_axiom(_content);
            _content->set_layout_parameters(_window_rectangle, _window_clipping_rectangle);
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->window_clipping_rectangle())) {
            draw_background(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    template<typename WidgetType = grid_layout_widget, typename... Args>
    std::shared_ptr<WidgetType> make_widget(Args &&... args) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto widget = super::make_widget<WidgetType>(std::forward<Args>(args)...);
        tt_axiom(!_content);
        _content = widget;
        return widget;
    }

private:
    std::shared_ptr<widget> _content;

    void draw_background(draw_context context) noexcept
    {
        context.clipping_rectangle = expand(context.clipping_rectangle, theme::global->borderWidth);
        context.draw_box_with_border_outside(rectangle(), foreground_color(), background_color());
    }
};

} // namespace tt