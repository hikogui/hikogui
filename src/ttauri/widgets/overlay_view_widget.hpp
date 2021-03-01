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
        }

        return has_updated_contraints;
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            tt_axiom(_content);
            _content->set_layout_parameters_from_parent(rectangle(), rectangle(), 1.0f);
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, _clipping_rectangle)) {
            draw_background(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    /** Make an overlay rectangle.
     * @param requested_rectangle A rectangle in the parent's local coordinate system.
     * @return A rectangle that fits the window's constraints in the parent's local coordinate system.
     */
    [[nodiscard]] aarect make_overlay_rectangle_from_parent(aarect requested_rectangle) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (auto parent = _parent.lock()) {
            ttlet requested_window_rectangle = aarect{parent->local_to_window() * requested_rectangle};
            ttlet window_bounds = aarect{10.0, 10.0, window.extent.width() - 20.0, window.extent.height() - 50.0};
            ttlet response_window_rectangle = fit(window_bounds, requested_window_rectangle);
            return aarect{parent->window_to_local() * response_window_rectangle};
        } else {
            tt_no_default();
        }
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
        context.draw_box_with_border_outside(rectangle(), background_color(), foreground_color());
    }
};

} // namespace tt