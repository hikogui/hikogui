// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/row_column_widget.hpp Defines row_column_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../GUI/theme.hpp"
#include "../geometry/module.hpp"
#include "../layout/row_column_layout.hpp"
#include <memory>
#include <type_traits>

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
     * @param parent The parent widget.
     */
    row_column_widget(widget *parent) noexcept : super(parent)
    {
        hi_axiom(loop::main().on_thread());

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
        auto tmp = std::make_unique<Widget>(this, std::forward<Args>(args)...);
        auto& ref = *tmp;
        _children.push_back(std::move(tmp));

        ++global_counter<"row_column_widget:make_widget:constrain">;
        process_event({gui_event_type::window_reconstrain});
        return ref;
    }

    /** Remove and deallocate all child widgets.
     */
    void clear() noexcept
    {
        hi_axiom(loop::main().on_thread());
        _children.clear();
        ++global_counter<"row_column_widget:clear:constrain">;
        process_event({gui_event_type::window_reconstrain});
    }

    /// @privatesection
    [[nodiscard]] generator<widget const &> children(bool include_invisible) const noexcept override
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

        return _children.constraints(os_settings::left_to_right());
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _children.set_layout(context.shape, theme().baseline_adjustment());

            for (hilet& child : _children) {
                child.value->set_layout(context.transform(child.shape, 0.0f));
            }
        }
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            for (hilet& child : _children) {
                child.value->draw(context);
            }
        }
    }

    hitbox hitbox_test(point2i position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial) {
            auto r = hitbox{};
            for (hilet& child : _children) {
                r = child.value->hitbox_test_from_parent(position, r);
            }
            return r;
        } else {
            return {};
        }
    }
    /// @endprivatesection
private:
    row_column_layout<Axis, std::unique_ptr<widget>> _children;
};

/** Lays out children in a row.
 * @ingroup widgets
 */
using row_widget = row_column_widget<axis::x>;

/** Lays out children in a column.
 * @ingroup widgets
 */
using column_widget = row_column_widget<axis::y>;

}} // namespace hi::v1
