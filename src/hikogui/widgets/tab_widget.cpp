// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "tab_widget.hpp"
#include "../scoped_buffer.hpp"

namespace hi::inline v1 {

tab_widget::~tab_widget()
{
    hi_assert_not_null(delegate);
    delegate->deinit(*this);
}

tab_widget::tab_widget(widget *parent, std::shared_ptr<delegate_type> delegate) noexcept :
    super(parent), delegate(std::move(delegate))
{
    hi_axiom(loop::main().on_thread());
    hi_assert_not_null(parent);

    // The tab-widget will not draw itself, only its selected child.
    semantic_layer = parent->semantic_layer;

    hi_assert_not_null(this->delegate);
    _delegate_cbt = this->delegate->subscribe([&] {
        ++global_counter<"tab_widget:delegate:constrain">;
        process_event({gui_event_type::window_reconstrain});
    });

    // Compare and assign would trigger the signaling NaN that widget sets.
    _constraints.minimum = {};
    _constraints.preferred = {};
    _constraints.maximum = {32767.0f, 32767.0f};
    hi_assert(_constraints.minimum <= _constraints.preferred && _constraints.preferred <= _constraints.maximum);

    this->delegate->init(*this);
}

widget_constraints const& tab_widget::set_constraints(set_constraints_context const& context) noexcept
{
    _layout = {};

    auto& selected_child_ = selected_child();

    if (_previous_selected_child != &selected_child_) {
        _previous_selected_child = &selected_child_;
        hi_log_info("tab_widget::set_constraints() selected tab changed");
        process_event({gui_event_type::window_resize});
    }

    for (hilet& child : _children) {
        child->mode = child.get() == &selected_child_ ? widget_mode::enabled : widget_mode::invisible;
    }

    return _constraints = selected_child_.set_constraints(context);
}

void tab_widget::set_layout(widget_layout const& context) noexcept
{
    _layout = context;

    for (hilet& child : _children) {
        if (*child->mode > widget_mode::invisible) {
            child->set_layout(context);
        }
    }
}

void tab_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible) {
        for (hilet& child : _children) {
            child->draw(context);
        }
    }
}

[[nodiscard]] hitbox tab_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial) {
        auto r = hitbox{};
        for (hilet& child : _children) {
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
    hi_axiom(loop::main().on_thread());
    return selected_child().find_next_widget(current_widget, group, direction);
}

[[nodiscard]] tab_widget::const_iterator tab_widget::find_selected_child() const noexcept
{
    hi_axiom(loop::main().on_thread());
    hi_assert_not_null(delegate);

    auto index = delegate->index(const_cast<tab_widget&>(*this));
    if (index >= 0 and index < ssize(_children)) {
        return _children.begin() + index;
    }

    return _children.end();
}

[[nodiscard]] widget& tab_widget::selected_child() const noexcept
{
    hi_axiom(loop::main().on_thread());
    hi_assert(not _children.empty());

    auto i = find_selected_child();
    if (i != _children.cend()) {
        return **i;
    } else {
        return *_children.front();
    }
}

} // namespace hi::inline v1
