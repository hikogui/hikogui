// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "GridLayoutWidget.hpp"
#include "../cells/ImageCell.hpp"
#include "../cells/TextCell.hpp"

namespace tt {

class overlay_view_widget final : public Widget {
public:
    overlay_view_widget(Window &window, Widget *parent) noexcept : Widget(window, parent)
    {
        if (parent) {
            // The overlay-widget will reset the semantic_layer as it is the bottom
            // layer of this virtual-window. However the draw-layer should be above
            // any other widget drawn.
            ttlet lock = std::scoped_lock(parent->mutex);
            p_draw_layer = parent->draw_layer() + 20.0f;
            p_semantic_layer = 0;
        }
    }

    ~overlay_view_widget() {}

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto has_updated_contraints = Widget::update_constraints();

        // Recurse into the selected widget.
        tt_assume(child);
        ttlet child_lock = std::scoped_lock(child->mutex);
        
        if (child->update_constraints() || has_updated_contraints) {
            p_preferred_size = child->preferred_size();
            p_preferred_base_line = child->preferred_base_line();
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(child);

        ttlet child_lock = std::scoped_lock(child->mutex);

        auto need_redraw = need_layout |= std::exchange(request_relayout, false);
        if (need_layout) {
            // The p_window_rectangle, is not allowed to be beyond the edges of the actual window.
            // Change p_window_rectangle to fit the window.
            ttlet window_rectangle_and_margin = expand(p_window_rectangle, p_margin);
            ttlet new_window_rectangle_and_margin = fit(aarect{window.currentWindowExtent}, window_rectangle_and_margin);
            p_window_rectangle = shrink(new_window_rectangle_and_margin, p_margin);
            p_window_clipping_rectangle = expand(p_window_rectangle, Theme::borderWidth);

            child->set_layout_parameters(p_window_rectangle, p_window_clipping_rectangle);
        }

        need_redraw |= child->update_layout(display_time_point, need_layout);
        return Widget::update_layout(display_time_point, need_layout) || need_redraw;
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(child);

        context.drawBoxExcludeBorder(rectangle());

        ttlet child_lock = std::scoped_lock(child->mutex);
        child->draw(child->make_draw_context(context), display_time_point);

        Widget::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        tt_assume(child);
        return child->hitbox_test(window_position);
    }

    Widget const *next_keyboard_widget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        tt_assume(child);
        return child->next_keyboard_widget(currentKeyboardWidget, reverse);
    }

    template<typename WidgetType = GridLayoutWidget, typename... Args>
    WidgetType &makeWidget(Args const &... args) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        auto widget_ptr = std::make_unique<WidgetType>(window, this, args...);
        auto &widget = *widget_ptr.get();
        child = std::move(widget_ptr);

        request_reconstrain = true;
        return widget;
    }

private:
    std::unique_ptr<Widget> child;
};

} // namespace tt