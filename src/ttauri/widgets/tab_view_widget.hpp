// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_container_widget.hpp"
#include "grid_layout_widget.hpp"

namespace tt {

template<typename T>
class tab_view_widget final : public abstract_container_widget {
public:
    using super = abstract_container_widget;
    using value_type = T;

    observable<value_type> value = 0;

    template<typename Value>
    tab_view_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent, Value &&value) noexcept :
        super(window, parent), value(std::forward<Value>(value))
    {
        if (parent) {
            // The tab-widget will not draw itself, only its selected child.
            ttlet lock = std::scoped_lock(gui_system_mutex);
            _draw_layer = parent->draw_layer();
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;

        _value_callback = value.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    ~tab_view_widget() {}

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto has_updated_contraints = super::update_constraints(display_time_point, need_reconstrain);
        if (has_updated_contraints) {

            ttlet &child = selected_child();
            if (compare_then_assign(_preferred_size, child.preferred_size())) {
                // The size of the selected child has changed, resize the window.
                window.requestResize = true;
            }
        }

        return has_updated_contraints;
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            for (auto &child : _children) {
                tt_axiom(child);
                child->set_layout_parameters_from_parent(rectangle());
            }
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        draw_child(context, display_time_point, selected_child());
        // Do not call super::draw, only the selected child should be drawn.
    }

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        ttlet &child = selected_child();
        return child.hitbox_test(point2{child.parent_to_local() * position});
    }

    std::shared_ptr<widget> find_next_widget(
        std::shared_ptr<widget> const &current_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return selected_child().find_next_widget(current_widget, group, direction);
    }

    template<typename WidgetType = grid_layout_widget, typename... Args>
    std::shared_ptr<WidgetType> make_widget(value_type value, Args &&... args) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto widget = super::make_widget<WidgetType>(std::forward<Args>(args)...);
        _children_keys.push_back(std::move(value));
        return widget;
    }

private:
    typename decltype(value)::callback_ptr_type _value_callback;

    std::vector<value_type> _children_keys;

    [[nodiscard]] auto find_child(value_type index) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
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
        tt_axiom(gui_system_mutex.recurse_lock_count());
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
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return find_child(*value);
    }

    [[nodiscard]] auto find_selected_child() noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return find_child(*value);
    }

    [[nodiscard]] widget const &selected_child() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(std::ssize(_children) != 0);

        auto i = find_selected_child();
        if (i != _children.cend()) {
            return *(*i);
        } else {
            return *_children.front();
        }
    }

    [[nodiscard]] widget &selected_child() noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(std::ssize(_children) != 0);

        auto i = find_selected_child();
        if (i != _children.cend()) {
            return *(*i);
        } else {
            return *_children.front();
        }
    }

    void draw_child(draw_context context, hires_utc_clock::time_point displayTimePoint, widget &child) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        child.draw(child.make_draw_context(context), displayTimePoint);
    }
};

} // namespace tt