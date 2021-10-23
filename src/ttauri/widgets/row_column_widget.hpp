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

namespace tt {

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
        window.request_reconstrain();
        return ref;
    }

    /** Remove and deallocate all child widgets.
     */
    void clear() noexcept
    {
        tt_axiom(is_gui_thread());
        _children.clear();
        window.request_reconstrain();
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        for (ttlet &child : _children) {
            co_yield child.get();
        }
    }

    [[nodiscard]] float margin() const noexcept override
    {
        return 0.0f;
    }

    widget_constraints const &set_constraints() noexcept override
    {
        tt_axiom(is_gui_thread());

        _layout = {};
        for (ttlet &child : _children) {
            child->set_constraints();
        }

        _flow_layout.clear();
        _flow_layout.reserve(std::ssize(_children));

        ssize_t index = 0;

        auto minimum_thickness = 0.0f;
        auto preferred_thickness = 0.0f;
        auto maximum_thickness = 0.0f;
        for (ttlet &child : _children) {
            update_constraints_for_child(*child, index++, minimum_thickness, preferred_thickness, maximum_thickness);
        }

        tt_axiom(index == std::ssize(_children));

        if constexpr (axis == axis::row) {
            _constraints.min = {_flow_layout.constraints().min, minimum_thickness};
            _constraints.pref = {_flow_layout.constraints().pref, preferred_thickness};
            _constraints.max = {_flow_layout.constraints().max, maximum_thickness};
        } else {
            _constraints.min = {minimum_thickness, _flow_layout.minimum_size()};
            _constraints.pref = {preferred_thickness, _flow_layout.preferred_size()};
            _constraints.max = {maximum_thickness, _flow_layout.maximum_size()};
        }
        tt_axiom(_constraints.min <= _constraints.pref && _constraints.pref <= _constraints.max);
        return _constraints;
    }

    void set_layout(widget_layout const &context) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (visible) {
            if (_layout.store(context) >= layout_update::size) {
                _flow_layout.set_size(axis == axis::row ? layout().width() : layout().height());
            }

            ssize_t index = 0;
            for (ttlet &child : _children) {
                update_layout_for_child(*child, index++, context);
            }

            tt_axiom(index == std::ssize(_children));
        }
    }

    void draw(draw_context const &context) noexcept override
    {
        if (visible and overlaps(context, layout())) {
            for (ttlet &child : _children) {
                child->draw(context);
            }
        }
    }

    /// @endprivatesection
private:
    std::vector<std::unique_ptr<widget>> _children;
    std::weak_ptr<delegate_type> _delegate;
    flow_layout _flow_layout;

    void update_constraints_for_child(
        widget const &child,
        ssize_t index,
        float &minimum_thickness,
        float &preferred_thickness,
        float &maximum_thickness) noexcept
    {
        tt_axiom(is_gui_thread());

        if (axis == axis::row) {
            ttlet minimum_length = child.constraints().min.width();
            ttlet preferred_length = child.constraints().pref.width();
            ttlet maximum_length = child.constraints().max.width();
            _flow_layout.update(index, minimum_length, preferred_length, maximum_length, child.margin());

            minimum_thickness = std::max(minimum_thickness, child.constraints().min.height() + child.margin() * 2.0f);
            preferred_thickness = std::max(preferred_thickness, child.constraints().pref.height() + child.margin() * 2.0f);
            maximum_thickness = std::max(maximum_thickness, child.constraints().max.height() + child.margin() * 2.0f);

        } else {
            ttlet minimum_length = child.constraints().min.height();
            ttlet preferred_length = child.constraints().pref.height();
            ttlet maximum_length = child.constraints().max.height();
            _flow_layout.update(index, minimum_length, preferred_length, maximum_length, child.margin());

            minimum_thickness = std::max(minimum_thickness, child.constraints().min.width() + child.margin() * 2.0f);
            preferred_thickness = std::max(preferred_thickness, child.constraints().pref.width() + child.margin() * 2.0f);
            maximum_thickness = std::max(maximum_thickness, child.constraints().max.width() + child.margin() * 2.0f);
        }
    }

    void update_layout_for_child(widget &child, ssize_t index, widget_layout const &context) const noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet[child_offset, child_length] = _flow_layout.get_offset_and_size(index++);

        ttlet child_rectangle = axis == axis::row ?
            aarectangle{
                child_offset,
                child.margin(),
                child_length,
                layout().height() - child.margin() * 2.0f} :
            aarectangle{
                child.margin(),
                layout().height() - child_offset - child_length,
                layout().width() - child.margin() * 2.0f,
                child_length};

        child.set_layout(child_rectangle * context);
    }
};

/** Lays out children in a row.
 */
using row_widget = row_column_widget<axis::row>;

/** Lays out children in a column.
 */
using column_widget = row_column_widget<axis::column>;

} // namespace tt
