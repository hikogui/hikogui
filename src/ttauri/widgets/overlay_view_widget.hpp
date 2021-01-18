// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"
#include "grid_layout_widget.hpp"

namespace tt {

class overlay_view_widget final : public widget {
public:
    using super = widget;

    overlay_view_widget(gui_window &window, std::shared_ptr<widget> parent) noexcept : super(window, parent)
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

        // Recurse into the selected widget.
        tt_axiom(child);
        if (child->update_constraints(display_time_point, need_reconstrain) || has_updated_contraints) {
            _preferred_size = child->preferred_size();
            _preferred_base_line = child->preferred_base_line();
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(child);

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            // The p_window_rectangle, is not allowed to be beyond the edges of the actual window.
            // Change p_window_rectangle to fit the window.
            ttlet window_rectangle_and_margin = expand(_window_rectangle, _margin);
            ttlet new_window_rectangle_and_margin = fit(aarect{f32x4{window.currentWindowExtent}}, window_rectangle_and_margin);
            _window_rectangle = shrink(new_window_rectangle_and_margin, _margin);
            _window_clipping_rectangle = _window_rectangle;

            child->set_layout_parameters(_window_rectangle, _window_clipping_rectangle);
        }

        child->update_layout(display_time_point, need_layout);
        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(child);

        if (overlaps(context, this->window_clipping_rectangle())) {
            draw_background(context);
        }

        child->draw(child->make_draw_context(context), display_time_point);
        super::draw(std::move(context), display_time_point);
    }

    bool handle_command_recursive(command command, std::vector<std::shared_ptr<widget>> const &reject_list) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(child);
        tt_axiom(child->parent.lock().get() == this);

        auto handled = false;
        handled |= child->handle_command_recursive(command, reject_list);
        handled |= super::handle_command_recursive(command, reject_list);
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(f32x4 window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        tt_axiom(child);
        return child->hitbox_test(window_position);
    }

    std::shared_ptr<widget> find_next_widget(
        std::shared_ptr<widget> const &currentKeyboardWidget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        tt_axiom(child);
        return child->find_next_widget(currentKeyboardWidget, group, direction);
    }

    template<typename WidgetType = grid_layout_widget, typename... Args>
    std::shared_ptr<WidgetType> make_widget(Args const &...args) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto widget = std::make_shared<WidgetType>(window, shared_from_this(), args...);
        widget->init();

        child = widget;
        _request_reconstrain = true;
        return widget;
    }

private:
    std::shared_ptr<widget> child;

    void draw_background(draw_context context) noexcept
    {
        context.clipping_rectangle = expand(context.clipping_rectangle, theme::global->borderWidth);
        context.draw_box_with_border_outside(rectangle());
    }
};

} // namespace tt