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

    [[nodiscard]] float margin() const noexcept override
    {
        return 0.0f;
    }

    [[nodiscard]] bool constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (super::constrain(display_time_point, need_reconstrain)) {
            _layout.clear();
            _layout.reserve(std::ssize(_children));

            ssize_t index = 0;

            auto minimum_thickness = 0.0f;
            auto preferred_thickness = 0.0f;
            auto maximum_thickness = 0.0f;
            for (ttlet &child : _children) {
                update_constraints_for_child(*child, index++, minimum_thickness, preferred_thickness, maximum_thickness);
            }

            tt_axiom(index == std::ssize(_children));

            if constexpr (axis == axis::row) {
                _minimum_size = {_layout.minimum_size(), minimum_thickness};
                _preferred_size = {_layout.preferred_size(), preferred_thickness};
                _maximum_size = {_layout.maximum_size(), maximum_thickness};
            } else {
                _minimum_size = {minimum_thickness, _layout.minimum_size()};
                _preferred_size = {preferred_thickness, _layout.preferred_size()};
                _maximum_size = {maximum_thickness, _layout.maximum_size()};
            }
            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;
        } else {
            return false;
        }
    }

    void layout(extent2 new_size, utc_nanoseconds display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(is_gui_thread());

        need_layout |= _relayout.exchange(false);
        if (need_layout) {
            _layout.set_size(axis == axis::row ? rectangle().width() : rectangle().height());

            ssize_t index = 0;
            for (ttlet &child : _children) {
                update_layout_for_child(*child, index++, display_time_point, need_layout);
            }

            tt_axiom(index == std::ssize(_children));
            request_redraw();
        }
    }
    /// @endprivatesection
private:
    std::vector<std::unique_ptr<widget>> _children;
    std::weak_ptr<delegate_type> _delegate;
    flow_layout _layout;

    void update_constraints_for_child(
        widget const &child,
        ssize_t index,
        float &minimum_thickness,
        float &preferred_thickness,
        float &maximum_thickness) noexcept
    {
        tt_axiom(is_gui_thread());

        if (axis == axis::row) {
            ttlet minimum_length = child.minimum_size().width();
            ttlet preferred_length = child.preferred_size().width();
            ttlet maximum_length = child.maximum_size().width();
            _layout.update(index, minimum_length, preferred_length, maximum_length, child.margin());

            minimum_thickness = std::max(minimum_thickness, child.minimum_size().height() + child.margin() * 2.0f);
            preferred_thickness = std::max(preferred_thickness, child.preferred_size().height() + child.margin() * 2.0f);
            maximum_thickness = std::max(maximum_thickness, child.maximum_size().height() + child.margin() * 2.0f);

        } else {
            ttlet minimum_length = child.minimum_size().height();
            ttlet preferred_length = child.preferred_size().height();
            ttlet maximum_length = child.maximum_size().height();
            _layout.update(index, minimum_length, preferred_length, maximum_length, child.margin());

            minimum_thickness = std::max(minimum_thickness, child.minimum_size().width() + child.margin() * 2.0f);
            preferred_thickness = std::max(preferred_thickness, child.preferred_size().width() + child.margin() * 2.0f);
            maximum_thickness = std::max(maximum_thickness, child.maximum_size().width() + child.margin() * 2.0f);
        }
    }

    void
    update_layout_for_child(widget &child, ssize_t index, utc_nanoseconds display_time_point, bool need_layout) const noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet[child_offset, child_length] = _layout.get_offset_and_size(index++);

        ttlet child_rectangle = axis == axis::row ?
            aarectangle{
                rectangle().left() + child_offset,
                rectangle().bottom() + child.margin(),
                child_length,
                rectangle().height() - child.margin() * 2.0f} :
            aarectangle{
                rectangle().left() + child.margin(),
                rectangle().top() - child_offset - child_length,
                rectangle().width() - child.margin() * 2.0f,
                child_length};

        child.set_layout_parameters_from_parent(child_rectangle);
        if (child.visible) {
            child.layout(child_rectangle.size(), display_time_point, need_layout);
        }
    }
};

/** Lays out children in a row.
 */
using row_widget = row_column_widget<axis::row>;

/** Lays out children in a column.
 */
using column_widget = row_column_widget<axis::column>;

} // namespace tt
