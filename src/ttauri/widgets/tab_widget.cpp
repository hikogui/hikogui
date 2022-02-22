// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "tab_widget.hpp"
#include "../scoped_buffer.hpp"

namespace tt::inline v1 {

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
    tt_axiom(parent);

    // The tab-widget will not draw itself, only its selected child.
    semantic_layer = parent->semantic_layer;

    if (auto d = _delegate.lock()) {
        d->subscribe(*this, _reconstrain_callback);
    }

    // Compare and assign would trigger the signaling NaN that widget sets.
    _constraints.minimum = {};
    _constraints.preferred = {};
    _constraints.maximum = {32767.0f, 32767.0f};
    tt_axiom(_constraints.minimum <= _constraints.preferred && _constraints.preferred <= _constraints.maximum);

    if (auto d = _delegate.lock()) {
        d->init(*this);
    }
}

tab_widget::tab_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    tab_widget(window, parent, weak_or_unique_ptr<delegate_type>{delegate})
{
}

widget_constraints const &tab_widget::set_constraints() noexcept
{
    _layout = {};

    auto &selected_child_ = selected_child();

    if (_previous_selected_child != &selected_child_) {
        _previous_selected_child = &selected_child_;
        request_resize();
    }

    for (ttlet &child : _children) {
        child->visible = child.get() == &selected_child_;
    }

    return _constraints = selected_child_.set_constraints();
}

void tab_widget::set_layout(widget_layout const &layout) noexcept
{
    _layout = layout;

    for (ttlet &child : _children) {
        if (child->visible) {
            child->set_layout(layout);
        }
    }
}

void tab_widget::draw(draw_context const &context) noexcept
{
    if (visible) {
        for (ttlet &child : _children) {
            child->draw(context);
        }
    }
}

[[nodiscard]] hitbox tab_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and enabled) {
        auto r = hitbox{};
        for (ttlet &child : _children) {
            r = child->hitbox_test_from_parent(position, r);
        }
        return r;
    } else {
        return {};
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

[[nodiscard]] tab_widget::const_iterator tab_widget::find_selected_child() const noexcept
{
    tt_axiom(is_gui_thread());
    if (auto delegate = _delegate.lock()) {
        auto index = delegate->index(const_cast<tab_widget &>(*this));
        if (index >= 0 and index < ssize(_children)) {
            return _children.begin() + index;
        }
    }
    return _children.end();
}

[[nodiscard]] widget &tab_widget::selected_child() const noexcept
{
    tt_axiom(is_gui_thread());
    tt_axiom(ssize(_children) != 0);

    auto i = find_selected_child();
    if (i != _children.cend()) {
        return **i;
    } else {
        return *_children.front();
    }
}

} // namespace tt::inline v1
