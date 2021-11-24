// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/// @file

#pragma once

#include "widget.hpp"
#include "row_column_delegate.hpp"
#include "../GUI/theme.hpp"
#include "../flow_layout.hpp"
#include "../geometry/axis.hpp"
#include <memory>

namespace tt::inline v1 {

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
 * @tparam Axis the axis to lay out child widgets. Either `axis::horizontal` or
 * `axis::vertical`.
 */
template<axis Axis>
class row_column_widget final : public widget {
public:
    static_assert(Axis == axis::horizontal or Axis == axis::vertical);

    using super = widget;
    using delegate_type = row_column_delegate<Axis>;
    static constexpr tt::axis axis = Axis;

    ~row_column_widget()
    {
        if (auto delegate = _delegate.lock()) {
            delegate->deinit(*this);
        }
    }

    /** Constructs an empty row/column widget.
     *
     * @param window The window.
     * @param parent The parent widget.
     * @param delegate An optional delegate can be used to populate the row/column widget
     *                 during initialization.
     */
    row_column_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate = {}) noexcept :
        super(window, parent), _delegate(std::move(delegate))
    {
        tt_axiom(is_gui_thread());

        if (parent) {
            semantic_layer = parent->semantic_layer;
        }
        if (auto d = _delegate.lock()) {
            d->init(*this);
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
    Widget &make_widget(Args &&...args)
    {
        auto tmp = std::make_unique<Widget>(window, this, std::forward<Args>(args)...);
        auto &ref = *tmp;
        _children.push_back(std::move(tmp));
        request_reconstrain();
        return ref;
    }

    /** Remove and deallocate all child widgets.
     */
    void clear() noexcept
    {
        tt_axiom(is_gui_thread());
        _children.clear();
        request_reconstrain();
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        for (ttlet &child : _children) {
            co_yield child.get();
        }
    }

    widget_constraints const &set_constraints() noexcept override
    {
        _layout = {};
        _flow_layout.clear();
        _flow_layout.reserve(ssize(_children));

        ssize_t index = 0;

        auto minimum_thickness = 0.0f;
        auto preferred_thickness = 0.0f;
        auto maximum_thickness = 0.0f;
        for (ttlet &child : _children) {
            update_constraints_for_child(*child, index++, minimum_thickness, preferred_thickness, maximum_thickness);
        }

        tt_axiom(index == ssize(_children));

        if constexpr (axis == axis::row) {
            return _constraints = {
                       {_flow_layout.minimum_size(), minimum_thickness},
                       {_flow_layout.preferred_size(), preferred_thickness},
                       {_flow_layout.maximum_size(), maximum_thickness}};
        } else {
            return _constraints = {
                       {minimum_thickness, _flow_layout.minimum_size()},
                       {preferred_thickness, _flow_layout.preferred_size()},
                       {maximum_thickness, _flow_layout.maximum_size()}};
        }
    }

    void set_layout(widget_layout const &layout) noexcept override
    {
        if (compare_store(_layout, layout)) {
            _flow_layout.set_size(axis == axis::row ? layout.width() : layout.height());
        }

        ssize_t index = 0;
        for (ttlet &child : _children) {
            update_layout_for_child(*child, index++, layout);
        }

        tt_axiom(index == ssize(_children));
    }

    void draw(draw_context const &context) noexcept override
    {
        if (visible) {
            for (ttlet &child : _children) {
                child->draw(context);
            }
        }
    }

    hitbox hitbox_test(point3 position) const noexcept override
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
    /// @endprivatesection
private:
    std::vector<std::unique_ptr<widget>> _children;
    std::weak_ptr<delegate_type> _delegate;
    flow_layout _flow_layout;

    void update_constraints_for_child(
        widget &child,
        ssize_t index,
        float &minimum_thickness,
        float &preferred_thickness,
        float &maximum_thickness) noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet &child_constraints = child.set_constraints();
        if (axis == axis::row) {
            _flow_layout.update(
                index,
                child_constraints.minimum.width(),
                child_constraints.preferred.width(),
                child_constraints.maximum.width(),
                child_constraints.margin);

            minimum_thickness = std::max(minimum_thickness, child_constraints.minimum.height() + child_constraints.margin * 2.0f);
            preferred_thickness =
                std::max(preferred_thickness, child_constraints.preferred.height() + child_constraints.margin * 2.0f);
            maximum_thickness = std::max(maximum_thickness, child_constraints.maximum.height() + child_constraints.margin * 2.0f);

        } else {
            _flow_layout.update(
                index,
                child_constraints.minimum.height(),
                child_constraints.preferred.height(),
                child_constraints.maximum.height(),
                child_constraints.margin);

            minimum_thickness = std::max(minimum_thickness, child_constraints.minimum.width() + child_constraints.margin * 2.0f);
            preferred_thickness =
                std::max(preferred_thickness, child_constraints.preferred.width() + child_constraints.margin * 2.0f);
            maximum_thickness = std::max(maximum_thickness, child_constraints.maximum.width() + child_constraints.margin * 2.0f);
        }
    }

    void update_layout_for_child(widget &child, ssize_t index, widget_layout const &context) const noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet[child_offset, child_length] = _flow_layout.get_offset_and_size(index++);

        ttlet &child_constraints = child.set_constraints();
        ttlet child_rectangle = axis == axis::row ?
            aarectangle{
                child_offset, child_constraints.margin, child_length, layout().height() - child_constraints.margin * 2.0f} :
            aarectangle{
                child_constraints.margin,
                layout().height() - child_offset - child_length,
                layout().width() - child_constraints.margin * 2.0f,
                child_length};

        child.set_layout(context.transform(child_rectangle, 0.0f));
    }
};

/** Lays out children in a row.
 */
using row_widget = row_column_widget<axis::row>;

/** Lays out children in a column.
 */
using column_widget = row_column_widget<axis::column>;

} // namespace tt::inline v1
