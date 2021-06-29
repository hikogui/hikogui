// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "grid_widget.hpp"
#include "tab_delegate.hpp"
#include "default_tab_delegate.hpp"

namespace tt {

class tab_widget final : public widget {
public:
    using super = widget;
    using delegate_type = tab_delegate;

    tab_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
        tab_widget(window, parent, weak_or_unique_ptr<delegate_type>{delegate})
    {
    }

    template<typename Value>
    requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>)
    tab_widget(gui_window &window, widget *parent, Value && value) noexcept :
        tab_widget(window, parent, make_unique_default_tab_delegate(std::forward<Value>(value)))
    {
    }

    ~tab_widget() {}

    void init() noexcept override
    {
        super::init();
        if (auto delegate = _delegate.lock()) {
            delegate->init(*this);
        }
    }

    void deinit() noexcept override
    {
        if (auto delegate = _delegate.lock()) {
            delegate->deinit(*this);
        }
        super::deinit();
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());

        auto has_updated_contraints = super::update_constraints(display_time_point, need_reconstrain);
        if (has_updated_contraints) {
            ttlet &selected_child_ = selected_child();
            for (ttlet &child : _children) {
                child->visible = child.get() == &selected_child_;
            }

            auto size_changed = compare_then_assign(_minimum_size, selected_child_.minimum_size());
            size_changed |= compare_then_assign(_preferred_size, selected_child_.preferred_size());
            size_changed |= compare_then_assign(_maximum_size, selected_child_.maximum_size());
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

    [[nodiscard]] widget const *find_next_widget(
        widget const *current_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept override
    {
        tt_axiom(is_gui_thread());
        return selected_child().find_next_widget(current_widget, group, direction);
    }

    template<typename WidgetType, typename Value, typename... Args>
    WidgetType &make_widget(Value const &value, Args &&...args) noexcept
    {
        tt_axiom(is_gui_thread());

        if (auto delegate = _delegate.lock()) {
            delegate->add_tab(*this, static_cast<size_t>(value), std::size(_children));
        }
        auto &widget = super::make_widget<WidgetType>(std::forward<Args>(args)...);
        return widget;
    }

private:
    weak_or_unique_ptr<delegate_type> _delegate;
    typename delegate_type::callback_ptr_type _delegate_callback;

    tab_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept :
        super(window, parent), _delegate(std::move(delegate))
    {
        tt_axiom(is_gui_thread());

        if (parent) {
            // The tab-widget will not draw itself, only its selected child.
            _draw_layer = parent->draw_layer();
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;

        if (auto d = _delegate.lock()) {
            _delegate_callback = d->subscribe(*this, [this](auto...) {
                this->_request_reconstrain = true;
            });
        }

        // Compare and assign would trigger the signaling NaN that widget sets.
        _minimum_size = {};
        _preferred_size = {};
        _maximum_size = {32767.0f, 32767.0f};
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
    }

    [[nodiscard]] auto find_selected_child() const noexcept
    {
        tt_axiom(is_gui_thread());
        if (auto delegate = _delegate.lock()) {
            auto index = delegate->index(const_cast<tab_widget &>(*this));
            if (index >= 0 and index < std::ssize(_children)) {
                return _children.begin() + index;
            }
        }
        return _children.end();
    }

    [[nodiscard]] auto find_selected_child() noexcept
    {
        tt_axiom(is_gui_thread());
        if (auto delegate = _delegate.lock()) {
            auto index = delegate->index(*this);
            if (index >= 0 and index < std::ssize(_children)) {
                return _children.begin() + index;
            }
        }
        return _children.end();
    }

    [[nodiscard]] widget const &selected_child() const noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(std::ssize(_children) != 0);

        auto i = find_selected_child();
        if (i != _children.cend()) {
            return **i;
        } else {
            return *_children.front();
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