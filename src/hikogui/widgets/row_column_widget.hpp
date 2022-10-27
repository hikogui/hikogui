// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/row_column_widget.hpp Defines row_column_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "grid_layout.hpp"
#include "../GUI/theme.hpp"
#include "../geometry/axis.hpp"
#include <memory>

namespace hi { inline namespace v1 {

/** A row/column widget lays out child widgets along a row or column.
 *
 * Columns are laid out from left to right, and rows from top to bottom.
 *
 * The row/column widget will calculate the size of the row or column based on
 * the minimum, preferred and maximum size of each child widget contained in
 * them. Margins are also taken into account in the spacing between the
 * child-widgets.
 *
 * When laid out, each child is sized to where it will occupy the full width of
 * a column, or full height of the row; and divide the length of the column or
 * row with the other children.
 *
 * @image html row_column_widget.png
 *
 * @ingroup widgets
 * @tparam Axis the axis to lay out child widgets. Either `axis::horizontal` or
 * `axis::vertical`.
 */
template<axis Axis>
class row_column_widget final : public widget {
public:
    static_assert(Axis == axis::horizontal or Axis == axis::vertical);

    using super = widget;
    static constexpr hi::axis axis = Axis;

    ~row_column_widget() {}

    /** Constructs an empty row/column widget.
     *
     * @param window The window.
     * @param parent The parent widget.
     */
    row_column_widget(gui_window& window, widget *parent) noexcept : super(window, parent)
    {
        hi_axiom(is_gui_thread());

        if (parent) {
            semantic_layer = parent->semantic_layer;
        }
    }

    /** Add a widget directly to this grid-widget.
     *
     * In a column-widget the newly added widget is added below previously added
     * child-widgets.
     *
     * In a row-widget the newly added widget is added to the right of
     * previously added child-widgets.
     *
     * @tparam Widget The type of the widget to be constructed.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget& make_widget(Args&&...args)
    {
        auto tmp = std::make_unique<Widget>(window, this, std::forward<Args>(args)...);
        auto& ref = *tmp;
        _children.push_back(std::move(tmp));
        hi_request_reconstrain("row_column_widget::make_widget()");
        return ref;
    }

    /** Remove and deallocate all child widgets.
     */
    void clear() noexcept
    {
        hi_axiom(is_gui_thread());
        _children.clear();
        hi_request_reconstrain("row_column_widget::clear()");
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override
    {
        for (hilet& child : _children) {
            co_yield child.get();
        }
    }

    widget_constraints const& set_constraints() noexcept override
    {
        _layout = {};

        ssize_t index = 0;

        auto minimum_thickness = 0.0f;
        auto preferred_thickness = 0.0f;
        auto maximum_thickness = 0.0f;
        float margin_before_thickness = 0.0f;
        float margin_after_thickness = 0.0f;
        widget_baseline baseline = {};

        _grid_layout.clear();
        for (hilet& child : _children) {
            update_constraints_for_child(
                *child,
                index++,
                minimum_thickness,
                preferred_thickness,
                maximum_thickness,
                margin_before_thickness,
                margin_after_thickness,
                baseline);
        }
        _grid_layout.commit_constraints();

        hi_assert(index == ssize(_children));

        if constexpr (axis == axis::row) {
            return _constraints = {
                       {_grid_layout.minimum(), minimum_thickness},
                       {_grid_layout.preferred(), preferred_thickness},
                       {_grid_layout.maximum(), maximum_thickness},
                       {_grid_layout.margin_before(),
                        margin_before_thickness,
                        _grid_layout.margin_after(),
                        margin_after_thickness},
                       baseline};
        } else {
            return _constraints = {
                       {minimum_thickness, _grid_layout.minimum()},
                       {preferred_thickness, _grid_layout.preferred()},
                       {maximum_thickness, _grid_layout.maximum()},
                       {margin_before_thickness,
                        _grid_layout.margin_before(),
                        margin_after_thickness,
                        _grid_layout.margin_after()}};
        }
    }

    void set_layout(widget_layout const& layout) noexcept override
    {
        if (compare_store(_layout, layout)) {
            _grid_layout.layout(axis == axis::row ? layout.width() : layout.height());
        }

        ssize_t index = 0;
        for (hilet& child : _children) {
            update_layout_for_child(*child, index++, layout);
        }

        hi_assert(index == ssize(_children));
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            for (hilet& child : _children) {
                child->draw(context);
            }
        }
    }

    hitbox hitbox_test(point3 position) const noexcept override
    {
        hi_axiom(is_gui_thread());

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
    /// @endprivatesection
private:
    std::vector<std::unique_ptr<widget>> _children;
    grid_layout _grid_layout;

    void update_constraints_for_child(
        widget& child,
        ssize_t index,
        float& minimum_thickness,
        float& preferred_thickness,
        float& maximum_thickness,
        float& margin_before_thickness,
        float& margin_after_thickness,
        widget_baseline& baseline) noexcept
    {
        hi_axiom(is_gui_thread());

        hilet& child_constraints = child.set_constraints();
        if (axis == axis::row) {
            _grid_layout.add_constraint(
                index,
                child_constraints.minimum.width(),
                child_constraints.preferred.width(),
                child_constraints.maximum.width(),
                child_constraints.margins.left(),
                child_constraints.margins.right());

            inplace_max(minimum_thickness, child_constraints.minimum.height());
            inplace_max(preferred_thickness, child_constraints.preferred.height());
            inplace_max(maximum_thickness, child_constraints.maximum.height());
            inplace_max(margin_before_thickness, child_constraints.margins.top());
            inplace_max(margin_after_thickness, child_constraints.margins.bottom());
            inplace_max(baseline, child_constraints.baseline);

        } else {
            _grid_layout.add_constraint(
                index,
                child_constraints.minimum.height(),
                child_constraints.preferred.height(),
                child_constraints.maximum.height(),
                child_constraints.margins.top(),
                child_constraints.margins.bottom(),
                child_constraints.baseline);

            inplace_max(minimum_thickness, child_constraints.minimum.width());
            inplace_max(preferred_thickness, child_constraints.preferred.width());
            inplace_max(maximum_thickness, child_constraints.maximum.width());
            inplace_max(margin_before_thickness, child_constraints.margins.left());
            inplace_max(margin_after_thickness, child_constraints.margins.right());
        }
    }

    void update_layout_for_child(widget& child, ssize_t index, widget_layout const& context) const noexcept
    {
        hi_axiom(is_gui_thread());

        hilet[child_position, child_length] = _grid_layout.get_position_and_size(index);

        if (axis == axis::row) {
            auto x0 = context.left_to_right() ? child_position : layout().width() - child_position - child_length;

            hilet child_rectangle = aarectangle{x0, 0.0f, child_length, layout().height()};
            // The baseline for a row widget is inherited from the context received from the parent.
            child.set_layout(context.transform(child_rectangle, 0.0f));

        } else {
            hilet child_rectangle =
                aarectangle{0.0f, layout().height() - child_position - child_length, layout().width(), child_length};
            child.set_layout(context.transform(child_rectangle, 0.0f, _grid_layout.get_baseline(index)));
        }
    }
};

/** Lays out children in a row.
 * @ingroup widgets
 */
using row_widget = row_column_widget<axis::row>;

/** Lays out children in a column.
 * @ingroup widgets
 */
using column_widget = row_column_widget<axis::column>;

}} // namespace hi::v1
