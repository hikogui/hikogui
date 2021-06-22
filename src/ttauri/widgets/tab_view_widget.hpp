// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "grid_layout_widget.hpp"

namespace tt {

template<typename T>
class tab_view_widget final : public widget {
public:
    using super = widget;
    using value_type = T;

    observable<value_type> value = 0;

    template<typename Value>
    tab_view_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value) noexcept :
        super(window, parent), value(std::forward<Value>(value))
    {
        tt_axiom(is_gui_thread());

        if (parent) {
            // The tab-widget will not draw itself, only its selected child.
            _draw_layer = parent->draw_layer();
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;

        _value_callback = value.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });

        // Compare and assign would trigger the signaling NaN that widget sets.
        _minimum_size = {};
        _preferred_size = {};
        _maximum_size = {32767.0f, 32767.0f};
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
    }

    ~tab_view_widget() {}

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());

        auto has_updated_contraints = super::update_constraints(display_time_point, need_reconstrain);
        if (has_updated_contraints) {
            ttlet &selected_child_ = selected_child();
            for (ttlet &child : _children) {
                child->visible = child == selected_child_;
            }

            auto size_changed = compare_then_assign(_minimum_size, selected_child_->minimum_size());
            size_changed |= compare_then_assign(_preferred_size, selected_child_->preferred_size());
            size_changed |= compare_then_assign(_maximum_size, selected_child_->maximum_size());
            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);

            if (size_changed) {
                window.requestResize = true;
            }
        }

        return has_updated_contraints;
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(is_gui_thread());

        need_layout |= _request_relayout.exchange(false);
        if (need_layout) {
            for (ttlet &child : _children) {
                if (child->visible) {
                    child->set_layout_parameters_from_parent(rectangle());
                }
            }
        }
        super::update_layout(display_time_point, need_layout);
    }

    std::shared_ptr<widget> find_next_widget(
        std::shared_ptr<widget> const &current_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept
    {
        tt_axiom(is_gui_thread());
        return selected_child()->find_next_widget(current_widget, group, direction);
    }

    template<typename WidgetType = grid_layout_widget, typename... Args>
    std::shared_ptr<WidgetType> make_widget(value_type value, Args &&...args) noexcept
    {
        tt_axiom(is_gui_thread());

        auto widget = super::make_widget<WidgetType>(std::forward<Args>(args)...);
        _children_keys.push_back(std::move(value));
        return widget;
    }

private:
    typename decltype(value)::callback_ptr_type _value_callback;

    std::vector<value_type> _children_keys;

    [[nodiscard]] auto find_child(value_type index) const noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(std::size(_children_keys) == std::size(_children));

        ttlet child_key_it = std::find(_children_keys.cbegin(), _children_keys.cend(), index);
        if (child_key_it != _children_keys.cend()) {
            ttlet child_index = std::distance(_children_keys.cbegin(), child_key_it);
            return _children.begin() + child_index;
        } else {
            return _children.cend();
        }
    }

    [[nodiscard]] auto find_child(value_type index) noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(std::size(_children_keys) == std::size(_children));

        ttlet child_key_it = std::find(_children_keys.cbegin(), _children_keys.cend(), index);
        if (child_key_it != _children_keys.cend()) {
            ttlet child_index = std::distance(_children_keys.cbegin(), child_key_it);
            return _children.cbegin() + child_index;
        } else {
            return _children.cend();
        }
    }

    [[nodiscard]] auto find_selected_child() const noexcept
    {
        tt_axiom(is_gui_thread());
        return find_child(*value);
    }

    [[nodiscard]] auto find_selected_child() noexcept
    {
        tt_axiom(is_gui_thread());
        return find_child(*value);
    }

    [[nodiscard]] std::shared_ptr<widget> const &selected_child() const noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(std::ssize(_children) != 0);

        auto i = find_selected_child();
        if (i != _children.cend()) {
            return *i;
        } else {
            return _children.front();
        }
    }

    void draw_child(draw_context context, hires_utc_clock::time_point displayTimePoint, widget &child) noexcept
    {
        tt_axiom(is_gui_thread());
        auto child_context =
            context.make_child_context(child.parent_to_local(), child.local_to_window(), child.clipping_rectangle());
        child.draw(child_context, displayTimePoint);
    }
};

} // namespace tt