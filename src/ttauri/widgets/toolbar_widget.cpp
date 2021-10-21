// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_widget.hpp"
#include "../scoped_buffer.hpp"

namespace tt {

toolbar_widget::toolbar_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    tt_axiom(is_gui_thread());

    if (parent) {
        // The toolbar is a top level widget, which draws its background as the next level.
        semantic_layer = 0;
    }
}

[[nodiscard]] float toolbar_widget::margin() const noexcept
{
    return 0.0f;
}

void toolbar_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    for (ttlet &child : _left_children) {
        child->constrain();
    }
    for (ttlet &child : _right_children) {
        child->constrain();
    }

    auto shared_height = 0.0f;

    _flow_layout.clear();
    _flow_layout.reserve(std::ssize(_left_children) + 1 + std::ssize(_right_children));

    ssize_t index = 0;
    for (ttlet &child : _left_children) {
        update_constraints_for_child(*child, index++, shared_height);
    }

    // Add a space between the left and right widgets.
    _flow_layout.update(index++, theme().large_size, theme().large_size, 32767.0f, 0.0f);

    for (ttlet &child : std::views::reverse(_right_children)) {
        update_constraints_for_child(*child, index++, shared_height);
    }

    tt_axiom(index == std::ssize(_left_children) + 1 + std::ssize(_right_children));
    _minimum_size = {_flow_layout.minimum_size(), shared_height};
    _preferred_size = {_flow_layout.preferred_size(), shared_height};
    _maximum_size = {_flow_layout.maximum_size(), shared_height};
    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void toolbar_widget::layout(layout_context const &context_) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        // Clip directly around the toolbar, so that tab buttons looks proper.
        ttlet context = context_.clip(context_.rectangle);
        if (compare_then_assign(_layout, context)) {
            request_redraw();
            _flow_layout.set_size(rectangle().width());
        }

        ssize_t index = 0;
        for (ttlet &child : _left_children) {
            update_layout_for_child(*child, index++, context);
        }

        // Skip over the cell between left and right children.
        index++;

        for (ttlet &child : std::views::reverse(_right_children)) {
            update_layout_for_child(*child, index++, context);
        }

        tt_axiom(index == std::ssize(_left_children) + 1 + std::ssize(_right_children));
    }
}

bool toolbar_widget::tab_button_has_focus() const noexcept
{
    for (ttlet &child : _left_children) {
        if (auto *c = dynamic_cast<toolbar_tab_button_widget *>(child.get())) {
            if (c->focus and c->state() == tt::button_state::on) {
                return true;
            }
        }
    }
    for (ttlet &child : _right_children) {
        if (auto *c = dynamic_cast<toolbar_tab_button_widget *>(child.get())) {
            if (c->focus and c->state() == tt::button_state::on) {
                return true;
            }
        }
    }

    return false;
}

void toolbar_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, _layout)) {
        context.draw_box(_layout, rectangle(), theme().color(theme_color::fill, semantic_layer + 1));

        if (tab_button_has_focus()) {
            ttlet line_rectangle =
                aarectangle{rectangle().left(), rectangle().bottom(), rectangle().width(), theme().border_width};

            // Draw the line at a higher elevation, so that the tab buttons can draw above or below the focus
            // line depending if that specific button is in focus or not.
            context.draw_box(_layout, translate_z(1.7f) * line_rectangle, focus_color());
        }

        for (ttlet &child : _left_children) {
            child->draw(context);
        }
        for (ttlet &child : _right_children) {
            child->draw(context);
        }
    }
}

hitbox toolbar_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    auto r = hitbox{};

    if (_layout.hit_rectangle.contains(position)) {
        r = hitbox{this, position, hitbox::Type::MoveArea};
    }

    auto buffer = pmr::scoped_buffer<256>{};
    for (auto *child : children(buffer.allocator())) {
        tt_axiom(child);
        r = std::max(r, child->hitbox_test(child->parent_to_local() * position));
    }
    return r;
}

void toolbar_widget::update_constraints_for_child(widget const &child, ssize_t index, float &shared_height) noexcept
{
    tt_axiom(is_gui_thread());

    _flow_layout.update(
        index, child.minimum_size().width(), child.preferred_size().width(), child.maximum_size().width(), child.margin());

    shared_height = std::max(shared_height, child.preferred_size().height() + child.margin() * 2.0f);
}

void toolbar_widget::update_layout_for_child(widget &child, ssize_t index, layout_context const &context) const noexcept
{
    tt_axiom(is_gui_thread());

    ttlet[child_x, child_width] = _flow_layout.get_offset_and_size(index++);

    ttlet child_rectangle = aarectangle{
        rectangle().left() + child_x,
        rectangle().bottom() + child.margin(),
        child_width,
        rectangle().height() - child.margin() * 2.0f};

    child.layout(child_rectangle * context);
}

widget &toolbar_widget::add_widget(horizontal_alignment alignment, std::unique_ptr<widget> widget) noexcept
{
    auto &ref = *widget;
    switch (alignment) {
        using enum horizontal_alignment;
    case left: _left_children.push_back(std::move(widget)); break;
    case right: _right_children.push_back(std::move(widget)); break;
    default: tt_no_default();
    }

    return ref;
}

[[nodiscard]] color toolbar_widget::focus_color() const noexcept
{
    if (enabled) {
        if (window.active) {
            return theme().color(theme_color::accent);
        } else if (hover) {
            return theme().color(theme_color::border, semantic_layer + 1);
        } else {
            return theme().color(theme_color::border, semantic_layer);
        }
    } else {
        return theme().color(theme_color::border, semantic_layer - 1);
    }
}

} // namespace tt
