// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "toolbar_widget.hpp"
#include "spacer_widget.hpp"
#include "toolbar_tab_button_widget.hpp"
#include "../scoped_buffer.hpp"
#include "../geometry/module.hpp"

namespace hi::inline v1 {

toolbar_widget::toolbar_widget(widget *parent) noexcept : super(parent)
{
    hi_axiom(loop::main().on_thread());

    // The toolbar is a top level widget, which draws its background as the next level.
    semantic_layer = 0;

    _children.push_back(std::make_unique<spacer_widget>(this));
}

[[nodiscard]] box_constraints toolbar_widget::update_constraints() noexcept
{
    _layout = {};

    for (auto& child : _children) {
        child.set_constraints(child.value->update_constraints());
    }

    auto r = _children.constraints(os_settings::left_to_right());
    _child_height_adjustment = -r.margins.top();

    r.minimum.height() += r.margins.top();
    r.preferred.height() += r.margins.top();
    r.maximum.height() += r.margins.top();
    r.padding.top() += r.margins.top();
    r.margins.top() = 0;

    return r;
}

void toolbar_widget::set_layout(widget_layout const& context) noexcept
{
    // Clip directly around the toolbar, so that tab buttons looks proper.
    if (compare_store(_layout, context)) {
        auto shape = context.shape;
        shape.rectangle = aarectanglei{shape.x(), shape.y(), shape.width(), shape.height() + _child_height_adjustment};
        _children.set_layout(shape, theme().baseline_adjustment());
    }

    hilet overhang = context.redraw_overhang;

    for (hilet& child : _children) {
        hilet child_clipping_rectangle =
            aarectanglei{child.shape.x() - overhang, 0, child.shape.width() + overhang * 2, context.height() + overhang * 2};

        child.value->set_layout(context.transform(child.shape, 1.0f, child_clipping_rectangle));
    }
}

bool toolbar_widget::tab_button_has_focus() const noexcept
{
    for (hilet& cell : _children) {
        if (auto const * const c = dynamic_cast<toolbar_tab_button_widget *>(cell.value.get())) {
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
            context.draw_box(
                layout(),
                layout().rectangle(),
                theme().color(semantic_color::fill, semantic_layer + 1));

            if (tab_button_has_focus()) {
                // Draw the line at a higher elevation, so that the tab buttons can draw above or below the focus
                // line depending if that specific button is in focus or not.
                hilet focus_rectangle = aarectanglei{0, 0, layout().rectangle().width(), theme().border_width()};
                context.draw_box(
                    layout(), translate3{0.0f, 0.0f, 1.5f} * narrow_cast<aarectangle>(focus_rectangle), focus_color());
            }
        }

        for (hilet& child : _children) {
            hi_assert_not_null(child.value);
            child.value->draw(context);
        }
    }
}

hitbox toolbar_widget::hitbox_test(point2i position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    // By default the toolbar is used for dragging the window.
    if (*mode >= widget_mode::partial) {
        auto r = layout().contains(position) ? hitbox{id, _layout.elevation, hitbox_type::move_area} : hitbox{};

        for (hilet& child : _children) {
            hi_assert_not_null(child.value);
            r = child.value->hitbox_test_from_parent(position, r);
        }

        return r;
    } else {
        return {};
    }
}

widget& toolbar_widget::add_widget(horizontal_alignment alignment, std::unique_ptr<widget> widget) noexcept
{
    auto& ref = *widget;
    switch (alignment) {
        using enum horizontal_alignment;
    case left:
        _children.insert(_children.cbegin() + _spacer_index, std::move(widget));
        ++_spacer_index;
        break;
    case right:
        _children.insert(_children.cbegin() + _spacer_index + 1, std::move(widget));
        break;
    default:
        hi_no_default();
    }

    return ref;
}

[[nodiscard]] color toolbar_widget::focus_color() const noexcept
{
    if (*mode >= widget_mode::partial) {
        return theme().color(semantic_color::accent);
    } else {
        return theme().color(semantic_color::border, semantic_layer - 1);
    }
}

} // namespace hi::inline v1
