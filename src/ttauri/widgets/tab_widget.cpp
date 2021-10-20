// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "tab_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../scoped_buffer.hpp"

namespace tt {

tab_widget::~tab_widget()
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

tab_widget::tab_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept :
    super(window, parent), _delegate(std::move(delegate))
{
    tt_axiom(is_gui_thread());

    if (parent) {
        // The tab-widget will not draw itself, only its selected child.
        semantic_layer = parent->semantic_layer;
    }

    if (auto d = _delegate.lock()) {
        _delegate_callback = d->subscribe(*this, [this](auto...) {
            this->request_reconstrain();
        });
    }

    // Compare and assign would trigger the signaling NaN that widget sets.
    _minimum_size = {};
    _preferred_size = {};
    _maximum_size = {32767.0f, 32767.0f};
    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);

        if (auto d = _delegate.lock()) {
        d->init(*this);
    }
}

tab_widget::tab_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    tab_widget(window, parent, weak_or_unique_ptr<delegate_type>{delegate})
{
}

[[nodiscard]] float tab_widget::margin() const noexcept
{
    return 0.0f;
}

[[nodiscard]] bool tab_widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    auto has_updated_contraints = super::constrain(display_time_point, need_reconstrain);
    if (has_updated_contraints) {
        ttlet &selected_child_ = selected_child();

        auto buffer = pmr::scoped_buffer<256>{};
        for (auto *child : children(buffer.allocator())) {
            tt_axiom(child);
            child->visible = child == &selected_child_;
        }

        auto size_changed = compare_then_assign(_minimum_size, selected_child_.minimum_size());
        size_changed |= compare_then_assign(_preferred_size, selected_child_.preferred_size());
        size_changed |= compare_then_assign(_maximum_size, selected_child_.maximum_size());
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);

        if (size_changed) {
            window.request_resize = true;
        }
    }

    return has_updated_contraints;
}

void tab_widget::layout(layout_context const &context, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    if (compare_then_assign(_layout, context) or need_layout) {
        auto buffer = pmr::scoped_buffer<256>{};
        for (auto *child : children(buffer.allocator())) {
            tt_axiom(child);
            if (child->visible) {
                child->layout(rectangle() * context, need_layout);
            }
        }
        request_redraw();
    }
}

[[nodiscard]] widget const *tab_widget::find_next_widget(
    widget const *current_widget,
    keyboard_focus_group group,
    keyboard_focus_direction direction) const noexcept
{
    tt_axiom(is_gui_thread());
    return selected_child().find_next_widget(current_widget, group, direction);
}

[[nodiscard]] auto tab_widget::find_selected_child() const noexcept
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

[[nodiscard]] auto tab_widget::find_selected_child() noexcept
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

[[nodiscard]] widget const &tab_widget::selected_child() const noexcept
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

} // namespace tt
