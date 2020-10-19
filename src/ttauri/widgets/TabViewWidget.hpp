// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "GridLayoutWidget.hpp"
#include "../cells/ImageCell.hpp"
#include "../cells/TextCell.hpp"

namespace tt {

class TabViewWidget final : public Widget {
public:
    observable<int> value = 0;

    template<typename V>
    TabViewWidget(Window &window, Widget *parent, V &&value) noexcept : Widget(window, parent), value(std::forward<V>(value))
    {
        if (parent) {
            // The tab-widget will not draw itself, only its selected child.
            ttlet lock = std::scoped_lock(parent->mutex);
            p_draw_layer = parent->draw_layer();
            p_semantic_layer = parent->semantic_layer();
        }
        p_margin = 0.0f;

        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...) {
            this->request_reconstrain = true;
        });
    }

    ~TabViewWidget() {}

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto has_updated_contraints = Widget::update_constraints();
        if (has_updated_contraints) {
            // The value has changed, so resize the window.
            window.requestResize = true;
        }

        // Recurse into the selected widget.
        tt_assume(*value >= 0 && *value < std::ssize(children));
        auto &child = children[*value];
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
        tt_assume(*value >= 0 && *value < std::ssize(children));

        auto &child = children[*value];
        ttlet child_lock = std::scoped_lock(child->mutex);

        auto need_redraw = need_layout |= std::exchange(request_relayout, false);
        if (need_layout) {
            child->set_layout_parameters(p_window_rectangle, p_window_clipping_rectangle, p_window_base_line);
        }

        need_redraw |= child->update_layout(display_time_point, need_layout);
        return Widget::update_layout(display_time_point, need_layout) || need_redraw;
    }

    void drawChild(DrawContext context, hires_utc_clock::time_point displayTimePoint, Widget &child) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        ttlet child_lock = std::scoped_lock(child.mutex);
        child.draw(child.make_draw_context(context), displayTimePoint);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(*value >= 0 && *value <= std::ssize(children));

        auto &child = children[*value];
        drawChild(context, display_time_point, *child);
        Widget::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        tt_assume(*value >= 0 && *value <= std::ssize(children));
        return children[*value]->hitbox_test(window_position);
    }

    Widget const *next_keyboard_widget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        tt_assume(*value >= 0 && *value < std::ssize(children));
        return children[*value]->next_keyboard_widget(currentKeyboardWidget, reverse);
    }

    template<typename WidgetType = GridLayoutWidget, typename... Args>
    WidgetType &addTab(Args const &... args) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        auto widget_ptr = std::make_unique<WidgetType>(window, this, args...);
        auto &widget = *widget_ptr.get();
        children.push_back(std::move(widget_ptr));

        // Make sure a tab is selected.
        if (*value < 0 || *value >= std::ssize(children)) {
            value = 0;
        }
        request_reconstrain = true;
        return widget;
    }

private:
    std::vector<std::unique_ptr<Widget>> children;
};

} // namespace tt