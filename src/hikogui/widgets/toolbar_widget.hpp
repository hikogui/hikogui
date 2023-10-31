// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toolbar_widget.hpp Defines toolbar_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../layout/layout.hpp"
#include "../geometry/geometry.hpp"
#include "../coroutine/coroutine.hpp"
#include "../macros.hpp"
#include <memory>
#include <ranges>
#include <coroutine>

hi_export_module(hikogui.widgets.toolbar_widget);

hi_export namespace hi { inline namespace v1 {

/** A toolbar widget is located at the top of a window and lays out its children
 * horizontally.
 *
 * The toolbar widget is shown at the top of a window, it is drawn with a
 * different background than the window's content to differentiate from it.
 *
 * Child widgets may be added on both the left side and right side of the
 * toolbar, a small space in the center is added for visual separation.
 *
 * Child widgets are drawn to full height of the toolbar determined by the
 * preferred size of all contained widgets. The width of each widget is
 * determined by their preferred size
 *
 * @ingroup widgets
 */
class toolbar_widget final : public widget {
public:
    using super = widget;

    /** Constructs an empty row/column widget.
     *
     * @param parent The parent widget.
     */
    toolbar_widget(not_null<widget_intf const *> parent) noexcept : super(parent)
    {
        hi_axiom(loop::main().on_thread());
        _children.push_back(std::make_unique<spacer_widget>(this));
    }

    /** Add a widget directly to this toolbar-widget.
     *
     * When `Alignment` is `horizontal_alignment::left` the new widget is added
     * to the right of the previous added widget on the left side of the
     * toolbar.
     *
     * When `Alignment` is `horizontal_alignment::right` the new widget is added
     * to the left of the previous added widget on the right side of the
     * toolbar.
     *
     * @tparam Widget The type of the widget to be constructed.
     * @tparam Alignment Add widgets to the left, or right.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, horizontal_alignment Alignment = horizontal_alignment::left, typename... Args>
    Widget& emplace(Args&&...args)
    {
        auto widget = std::make_unique<Widget>(this, std::forward<Args>(args)...);
        return static_cast<Widget&>(insert(Alignment, std::move(widget)));
    }

    /// @privatesection
    [[nodiscard]] generator<widget_intf &> children(bool include_invisible) noexcept override
    {
        for (hilet& child : _children) {
            co_yield *child.value;
        }
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
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
    void set_layout(widget_layout const& context) noexcept override
    {
        // Clip directly around the toolbar, so that tab buttons looks proper.
        if (compare_store(_layout, context)) {
            auto shape = context.shape;
            shape.rectangle = aarectangle{shape.x(), shape.y(), shape.width(), shape.height() + _child_height_adjustment};
            _children.set_layout(shape, theme().baseline_adjustment());
        }

        hilet overhang = context.redraw_overhang;

        for (hilet& child : _children) {
            hilet child_clipping_rectangle =
                aarectangle{child.shape.x() - overhang, 0, child.shape.width() + overhang * 2, context.height() + overhang * 2};

            child.value->set_layout(context.transform(child.shape, transform_command::menu_item, child_clipping_rectangle));
        }
    }
    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            if (overlaps(context, layout())) {
                context.draw_box(layout(), layout().rectangle(), theme().color(semantic_color::fill, _layout.layer));

                if (tab_button_has_focus()) {
                    // Draw the line at a higher elevation, so that the tab buttons can draw above or below the focus
                    // line depending if that specific button is in focus or not.
                    hilet focus_rectangle = aarectangle{0.0f, 0.0f, layout().rectangle().width(), theme().border_width()};
                    context.draw_box(layout(), translate3{0.0f, 0.0f, 1.5f} * focus_rectangle, focus_color());
                }
            }

            for (hilet& child : _children) {
                hi_assert_not_null(child.value);
                child.value->draw(context);
            }
        }
    }
    hitbox hitbox_test(point2 position) const noexcept override
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
    [[nodiscard]] color focus_color() const noexcept override
    {
        if (*mode >= widget_mode::partial) {
            return theme().color(semantic_color::accent);
        } else {
            return theme().color(semantic_color::border, _layout.layer - 1);
        }
    }
    /// @endprivatesection
private:
    mutable row_layout<std::unique_ptr<widget>> _children;
    mutable float _child_height_adjustment = 0.0f;
    size_t _spacer_index = 0;

    void update_layout_for_child(widget& child, ssize_t index, widget_layout const& context) const noexcept;

    /** Add a widget directly to this widget.
     */
    widget& insert(horizontal_alignment alignment, std::unique_ptr<widget> widget) noexcept
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

    /** Check if a child tab-button has focus.
     *
     * @return If true the toolbar should draw a focus bar.
     */
    bool tab_button_has_focus() const noexcept
    {
        for (hilet& cell : _children) {
            if (auto const *const c = dynamic_cast<toolbar_tab_button_widget *>(cell.value.get())) {
                if (*c->focus and c->state() == hi::button_state::on) {
                    return true;
                }
            }
        }

        return false;
    }
};

}} // namespace hi::v1
