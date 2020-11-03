// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "GridLayoutWidget.hpp"

namespace tt {

template<typename T>
class tab_view_widget final : public widget {
public:
    using value_type = T;

    observable<value_type> value = 0;

    template<typename Value>
    tab_view_widget(Window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        widget(window, parent), value(std::forward<Value>(value))
    {
        if (parent) {
            // The tab-widget will not draw itself, only its selected child.
            ttlet lock = std::scoped_lock(GUISystem_mutex);
            _draw_layer = parent->draw_layer();
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;

        _value_callback = value.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    ~tab_view_widget() {}

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        auto has_updated_contraints = widget::update_constraints();
        if (has_updated_contraints) {
            // The value has changed, so resize the window.
            window.requestResize = true;
        }

        // Recurse into the selected widget.
        auto &child = selected_child();
        
        if (child.update_constraints() || has_updated_contraints) {
            _preferred_size = child.preferred_size();
            _preferred_base_line = child.preferred_base_line();
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        auto &child = selected_child();

        auto need_redraw = need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            child.set_layout_parameters(_window_rectangle, _window_clipping_rectangle, _window_base_line);
        }

        need_redraw |= child.update_layout(display_time_point, need_layout);
        return widget::update_layout(display_time_point, need_layout) || need_redraw;
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        draw_child(context, display_time_point, selected_child());
        widget::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        return selected_child().hitbox_test(window_position);
    }

    std::shared_ptr<widget> next_keyboard_widget(std::shared_ptr<widget> const &currentKeyboardWidget, bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        return selected_child().next_keyboard_widget(currentKeyboardWidget, reverse);
    }

    template<typename WidgetType = GridLayoutWidget, typename... Args>
    std::shared_ptr<WidgetType> make_widget(value_type value, Args const &... args) noexcept
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);

        auto widget = std::make_shared<WidgetType>(window, shared_from_this(), args...);
        widget->initialize();

        _children.emplace_back(std::move(value), widget);
        _request_reconstrain = true;
        return widget;
    }

private:
    typename decltype(value)::callback_ptr_type _value_callback;

    std::vector<std::pair<value_type, std::shared_ptr<widget>>> _children;
    using const_iterator = typename decltype(_children)::const_iterator;
    using iterator = typename decltype(_children)::iterator;

    [[nodiscard]] const_iterator find_child(value_type index) const noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return std::find_if(_children.cbegin(), _children.cend(), [&index](ttlet &x) {
            return x.first == index;
        });
    }

    [[nodiscard]] iterator find_child(value_type index) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return std::find_if(_children.begin(), _children.end(), [&index](ttlet &x) {
            return x.first == index;
        });
    }

    [[nodiscard]] const_iterator find_selected_child() const noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return find_child(*value);
    }

    [[nodiscard]] iterator find_selected_child() noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return find_child(*value);
    }

    [[nodiscard]] widget const &selected_child() const noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        tt_assume(std::ssize(_children) != 0);

        auto i = find_selected_child();
        if (i != _children.cend()) {
            return *i->second;
        } else {
            return *_children.front().second;
        }
    }

    [[nodiscard]] widget &selected_child() noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        tt_assume(std::ssize(_children) != 0);

        auto i = find_selected_child();
        if (i != _children.cend()) {
            return *i->second;
        } else {
            return *_children.front().second;
        }
    }

    void draw_child(DrawContext context, hires_utc_clock::time_point displayTimePoint, widget &child) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        child.draw(child.make_draw_context(context), displayTimePoint);
    }
};

} // namespace tt