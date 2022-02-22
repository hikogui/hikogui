// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_widget.hpp"
#include "toolbar_tab_button_widget.hpp"
#include "../scoped_buffer.hpp"
#include "../geometry/translate.hpp"

namespace tt::inline v1 {

toolbar_widget::toolbar_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    tt_axiom(is_gui_thread());

    if (parent) {
        // The toolbar is a top level widget, which draws its background as the next level.
        semantic_layer = 0;
    }
}

widget_constraints const &toolbar_widget::set_constraints() noexcept
{
    _layout = {};
    _grid_layout.clear();
    ssize_t index = 0;
    auto shared_height = 0.0f;
    auto shared_top_margin = 0.0f;
    auto shared_bottom_margin = 0.0f;
    for (ttlet &child : _left_children) {
        update_constraints_for_child(*child, index++, shared_height, shared_top_margin, shared_bottom_margin);
    }

    // Add a space between the left and right widgets.
    _grid_layout.add_constraint(index++, theme().large_size, theme().large_size, 32767.0f, 0.0f, 0.0f);

    for (ttlet &child : std::views::reverse(_right_children)) {
        update_constraints_for_child(*child, index++, shared_height, shared_top_margin, shared_bottom_margin);
    }

    tt_axiom(index == ssize(_left_children) + 1 + ssize(_right_children));

    _grid_layout.commit_constraints();
    _inner_margins = {0.0f, shared_bottom_margin, 0.0f, shared_top_margin};

    // The toolbar shows a background color that envelops the margins of the toolbar-items.
    // The toolbar itself only has a bottom margin.
    ttlet minimum_width = _grid_layout.minimum() + _grid_layout.margin_before() + _grid_layout.margin_after();
    ttlet preferred_width = _grid_layout.preferred() + _grid_layout.margin_before() + _grid_layout.margin_after();
    ttlet maximum_width = _grid_layout.maximum() + _grid_layout.margin_before() + _grid_layout.margin_after();
    ttlet height = shared_height + _inner_margins.top() + _inner_margins.bottom();

    return _constraints = {{minimum_width, height}, {preferred_width, height}, {maximum_width, height}};
}

void toolbar_widget::set_layout(widget_layout const &layout) noexcept
{
    // Clip directly around the toolbar, so that tab buttons looks proper.
    if (compare_store(_layout, layout)) {
        _grid_layout.layout(layout.width());
    }

    ssize_t index = 0;
    for (ttlet &child : _left_children) {
        update_layout_for_child(*child, index++, layout);
    }

    // Skip over the cell between left and right children.
    index++;

    for (ttlet &child : std::views::reverse(_right_children)) {
        update_layout_for_child(*child, index++, layout);
    }

    tt_axiom(index == ssize(_left_children) + 1 + ssize(_right_children));
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
    if (visible) {
        if (overlaps(context, layout())) {
            context.draw_box(layout(), layout().rectangle(), theme().color(theme_color::fill, semantic_layer + 1));

            if (tab_button_has_focus()) {
                // Draw the line at a higher elevation, so that the tab buttons can draw above or below the focus
                // line depending if that specific button is in focus or not.
                ttlet focus_rectangle = aarectangle{0.0, 0.0, layout().rectangle().width(), theme().border_width};
                context.draw_box(layout(), translate3{0.0f, 0.0f, 1.5f} * focus_rectangle, focus_color());
            }
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

    // By default the toolbar is used for dragging the window.
    if (visible and enabled) {
        auto r = layout().contains(position) ? hitbox{this, position, hitbox::Type::MoveArea} : hitbox{};

        for (ttlet &child : _left_children) {
            tt_axiom(child);
            r = child->hitbox_test_from_parent(position, r);
        }

        for (ttlet &child : _right_children) {
            tt_axiom(child);
            r = child->hitbox_test_from_parent(position, r);
        }

        return r;
    } else {
        return {};
    }
}

void toolbar_widget::update_constraints_for_child(
    widget &child,
    ssize_t index,
    float &shared_height,
    float &shared_top_margin,
    float &shared_bottom_margin) noexcept
{
    tt_axiom(is_gui_thread());

    ttlet child_constraints = child.set_constraints();
    _grid_layout.add_constraint(
        index,
        child_constraints.minimum.width(),
        child_constraints.preferred.width(),
        child_constraints.maximum.width(),
        child_constraints.margins.left(),
        child_constraints.margins.right());

    inplace_max(shared_height, child_constraints.preferred.height());
    inplace_max(shared_top_margin, child_constraints.margins.top());
    inplace_max(shared_bottom_margin, child_constraints.margins.bottom());
}

void toolbar_widget::update_layout_for_child(widget &child, ssize_t index, widget_layout const &context) const noexcept
{
    tt_axiom(is_gui_thread());

    ttlet[child_x, child_width] = _grid_layout.get_position_and_size(index);

    ttlet child_rectangle = aarectangle{
        child_x, _inner_margins.bottom(), child_width, layout().height() - _inner_margins.bottom() - _inner_margins.top()};

    ttlet child_clipping_rectangle = aarectangle{
        child_x - child.constraints().margins.left(),
        0.0f,
        child_width + child.constraints().margins.left() + child.constraints().margins.right(),
        layout().height()};
    child.set_layout(context.transform(child_rectangle, 1.0f, child_clipping_rectangle));
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
        return theme().color(theme_color::accent);
    } else {
        return theme().color(theme_color::border, semantic_layer - 1);
    }
}

} // namespace tt::inline v1
