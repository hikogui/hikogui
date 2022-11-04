// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_widget.hpp"
#include "toolbar_tab_button_widget.hpp"
#include "../scoped_buffer.hpp"
#include "../geometry/translate.hpp"

namespace hi::inline v1 {

toolbar_widget::toolbar_widget(widget *parent) noexcept : super(parent)
{
    hi_axiom(loop::main().on_thread());

    if (parent) {
        // The toolbar is a top level widget, which draws its background as the next level.
        semantic_layer = 0;
    }
}

widget_constraints const& toolbar_widget::set_constraints(set_constraints_context const &context) noexcept
{
    _layout = {};
    _grid_layout.clear();
    ssize_t index = 0;
    auto shared_height = 0.0f;
    auto shared_top_margin = 0.0f;
    auto shared_bottom_margin = 0.0f;
    auto shared_baseline = widget_baseline{};
    for (hilet& child : _left_children) {
        update_constraints_for_child(context, *child, index++, shared_height, shared_top_margin, shared_bottom_margin, shared_baseline);
    }

    // Add a space between the left and right widgets.
    _grid_layout.add_constraint(index++, context.theme->large_size, context.theme->large_size, 32767.0f, 0.0f, 0.0f);

    for (hilet& child : std::views::reverse(_right_children)) {
        update_constraints_for_child(context, *child, index++, shared_height, shared_top_margin, shared_bottom_margin, shared_baseline);
    }

    hi_assert(index == ssize(_left_children) + 1 + ssize(_right_children));

    _grid_layout.commit_constraints();
    _inner_margins = {0.0f, shared_bottom_margin, 0.0f, shared_top_margin};

    // The toolbar shows a background color that envelops the margins of the toolbar-items.
    // The toolbar itself only has a bottom margin.
    hilet minimum_width = _grid_layout.minimum() + _grid_layout.margin_before() + _grid_layout.margin_after();
    hilet preferred_width = _grid_layout.preferred() + _grid_layout.margin_before() + _grid_layout.margin_after();
    hilet maximum_width = _grid_layout.maximum() + _grid_layout.margin_before() + _grid_layout.margin_after();
    hilet height = shared_height + _inner_margins.top() + _inner_margins.bottom();

    return _constraints = {
               {minimum_width, height}, {preferred_width, height}, {maximum_width, height}, margins{}, shared_baseline};
}

void toolbar_widget::set_layout(widget_layout const& context) noexcept
{
    // Clip directly around the toolbar, so that tab buttons looks proper.
    if (compare_store(_layout, context)) {
        _grid_layout.layout(context.width());
    }

    ssize_t index = 0;
    for (hilet& child : _left_children) {
        update_layout_for_child(*child, index++, context);
    }

    // Skip over the cell between left and right children.
    index++;

    for (hilet& child : std::views::reverse(_right_children)) {
        update_layout_for_child(*child, index++, context);
    }

    hi_assert(index == ssize(_left_children) + 1 + ssize(_right_children));
}

bool toolbar_widget::tab_button_has_focus() const noexcept
{
    for (hilet& child : _left_children) {
        if (auto *c = dynamic_cast<toolbar_tab_button_widget *>(child.get())) {
            if (*c->focus and c->state() == hi::button_state::on) {
                return true;
            }
        }
    }
    for (hilet& child : _right_children) {
        if (auto *c = dynamic_cast<toolbar_tab_button_widget *>(child.get())) {
            if (*c->focus and c->state() == hi::button_state::on) {
                return true;
            }
        }
    }

    return false;
}

void toolbar_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible) {
        if (overlaps(context, layout())) {
            context.draw_box(layout(), layout().rectangle(), layout().theme->color(semantic_color::fill, semantic_layer + 1));

            if (tab_button_has_focus()) {
                // Draw the line at a higher elevation, so that the tab buttons can draw above or below the focus
                // line depending if that specific button is in focus or not.
                hilet focus_rectangle = aarectangle{0.0, 0.0, layout().rectangle().width(), layout().theme->border_width};
                context.draw_box(layout(), translate3{0.0f, 0.0f, 1.5f} * focus_rectangle, focus_color());
            }
        }

        for (hilet& child : _left_children) {
            child->draw(context);
        }
        for (hilet& child : _right_children) {
            child->draw(context);
        }
    }
}

hitbox toolbar_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    // By default the toolbar is used for dragging the window.
    if (*mode >= widget_mode::partial) {
        auto r = layout().contains(position) ? hitbox{this, position, hitbox_type::move_area} : hitbox{};

        for (hilet& child : _left_children) {
            hi_assert_not_null(child);
            r = child->hitbox_test_from_parent(position, r);
        }

        for (hilet& child : _right_children) {
            hi_assert_not_null(child);
            r = child->hitbox_test_from_parent(position, r);
        }

        return r;
    } else {
        return {};
    }
}

void toolbar_widget::update_constraints_for_child(
    set_constraints_context const &context,
    widget& child,
    ssize_t index,
    float& shared_height,
    float& shared_top_margin,
    float& shared_bottom_margin,
    widget_baseline &shared_baseline) noexcept
{
    hi_axiom(loop::main().on_thread());

    hilet child_constraints = child.set_constraints(context);
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
    inplace_max(shared_baseline, child_constraints.baseline);
}

void toolbar_widget::update_layout_for_child(widget& child, ssize_t index, widget_layout const& context) const noexcept
{
    hi_axiom(loop::main().on_thread());

    hilet[child_offset, child_width] = _grid_layout.get_position_and_size(index);

    hilet x0 = context.left_to_right() ? child_offset : context.width() - child_offset - child_width;

    hilet child_rectangle = aarectangle{x0, _inner_margins.bottom(), child_width, context.height() - _inner_margins.bottom() - _inner_margins.top()};

    hilet margin_left = context.left_to_right() ? child.constraints().margins.left() : child.constraints().margins.right();
    hilet margin_right = context.left_to_right() ? child.constraints().margins.right() : child.constraints().margins.left();

    hilet child_clipping_rectangle = aarectangle{
        x0 - margin_left,
        0.0f,
        child_width + margin_left + margin_right, context.height()};
    child.set_layout(context.transform(child_rectangle, 1.0f, child_clipping_rectangle));
}

widget& toolbar_widget::add_widget(horizontal_alignment alignment, std::unique_ptr<widget> widget) noexcept
{
    auto& ref = *widget;
    switch (alignment) {
        using enum horizontal_alignment;
    case left:
        _left_children.push_back(std::move(widget));
        break;
    case right:
        _right_children.push_back(std::move(widget));
        break;
    default:
        hi_no_default();
    }

    return ref;
}

[[nodiscard]] color toolbar_widget::focus_color() const noexcept
{
    if (*mode >= widget_mode::partial) {
        return layout().theme->color(semantic_color::accent);
    } else {
        return layout().theme->color(semantic_color::border, semantic_layer - 1);
    }
}

} // namespace hi::inline v1
