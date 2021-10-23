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
        d->subscribe(*this, _reconstrain_callback);
    }

    // Compare and assign would trigger the signaling NaN that widget sets.
    _constraints.min = {};
    _constraints.pref = {};
    _constraints.max = {32767.0f, 32767.0f};
    tt_axiom(_constraints.min <= _constraints.pref && _constraints.pref <= _constraints.max);

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

widget_constraints const &tab_widget::set_constraints() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    for (ttlet &child : _children) {
        child->set_constraints();
    }

    ttlet &selected_child_ = selected_child();

    auto buffer = pmr::scoped_buffer<256>{};
    for (auto *child : children(buffer.allocator())) {
        tt_axiom(child);
        child->visible = child == &selected_child_;
    }

    auto size_changed = compare_then_assign(_constraints.min, selected_child_.constraints().min);
    size_changed |= compare_then_assign(_constraints.pref, selected_child_.constraints().pref);
    size_changed |= compare_then_assign(_constraints.max, selected_child_.constraints().max);
    tt_axiom(_constraints.min <= _constraints.pref && _constraints.pref <= _constraints.max);

    if (size_changed) {
        window.request_resize = true;
    }
    return _constraints;
}

void tab_widget::set_layout(widget_layout const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        _layout.store(context);

        for (ttlet &child : _children) {
            child->set_layout(layout().rectangle() * context);
        }
    }
}

void tab_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        for (ttlet &child : _children) {
            child->draw(context);
        }
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
