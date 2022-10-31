// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/grid_widget.hpp Defines grid_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "grid_layout.hpp"
#include "../geometry/spreadsheet_address.hpp"
#include <memory>

namespace hi { inline namespace v1 {

/** A GUI widget that lays out child-widgets in a grid with variable sized cells.
 * @ingroup widgets
 *
 * The grid widget lays out child widgets in a grid pattern. Each child widget
 * occupies a single cell, which belongs into a single column and a single row.
 *
 * Columns are laid out from left to right, and rows from top to bottom. The row
 * and columns number may be specified as integers, or using an spreadsheet-like
 * cell-address:
 *  - `grid_widget::make_widget<T>(std::size_t column_nr, std::size_t row_nr, ...)`
 *  - `grid_widget::make_widget<T>(std::string address, ...)`
 *
 * The grid widget will calculate the size of each row and column based on the
 * minimum, preferred and maximum size of each child widget contained in them.
 * Margins are also taken into account in the spacing between columns and
 * between rows.
 *
 * When laid out, each child is sized to where it will occupy the full width and
 * height of each cell.
 *
 * @image html grid_widget.png
 */
class grid_widget : public widget {
public:
    using super = widget;

    ~grid_widget();

    /** Constructs an empty grid widget.
     *
     * @param parent The parent widget.
     */
    grid_widget(widget *parent) noexcept;

    /** Add a widget directly to this grid-widget.
     *
     * @tparam Widget The type of the widget to be constructed.
     * @param column_first The first (left-most) column to place the widget in.
     * @param row_first The first (top-most) row to place the widget in.
     * @param column_last One beyond the last column to place the widget in.
     * @param row_last One beyond the last row to place the widget in.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget&
    make_widget(std::size_t column_first, std::size_t row_first, std::size_t column_last, std::size_t row_last, Args&&...args)
    {
        auto tmp = std::make_unique<Widget>(this, std::forward<Args>(args)...);
        return static_cast<Widget&>(add_widget(column_first, row_first, column_last, row_last, std::move(tmp)));
    }

    /** Add a widget directly to this grid-widget.
     *
     * @tparam Widget The type of the widget to be constructed.
     * @param column The zero-based index from left-to-right.
     * @param row The zero-based index from top-to-bottom.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget& make_widget(std::size_t column, std::size_t row, Args&&...args)
    {
        auto tmp = std::make_unique<Widget>(this, std::forward<Args>(args)...);
        return static_cast<Widget&>(add_widget(column, row, column + 1, row + 1, std::move(tmp)));
    }

    /** Add a widget directly to this grid-widget.
     *
     * @tparam Widget The type of the widget to be constructed.
     * @param address The spreadsheet-like address of the cell,
     *                see `parse_spreadsheet_address()`.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget& make_widget(std::string_view address, Args&&...args)
    {
        hilet[column_first, row_first, column_last, row_last] = parse_spreadsheet_range(address);
        return make_widget<Widget>(column_first, row_first, column_last, row_last, std::forward<Args>(args)...);
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override
    {
        for (hilet& cell : _cells) {
            co_yield cell.widget.get();
        }
    }

    widget_constraints const& set_constraints(set_constraints_context const& context) noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    /// @endprivatesection
private:
    struct cell_type {
        std::size_t column_first;
        std::size_t row_first;
        std::size_t column_last;
        std::size_t row_last;
        std::unique_ptr<hi::widget> widget;

        cell_type(
            std::size_t column_first,
            std::size_t row_first,
            std::size_t column_last,
            std::size_t row_last,
            std::unique_ptr<hi::widget> widget) noexcept :
            column_first(column_first),
            row_first(row_first),
            column_last(column_last),
            row_last(row_last),
            widget(std::move(widget))
        {
        }

        [[nodiscard]] aarectangle
        rectangle(grid_layout const& columns, grid_layout const& rows, extent2 container_size, bool ltor) const noexcept
        {
            hilet[x_min, x_max] = columns.get_positions(column_first, column_last);
            hilet[y_min, y_max] = rows.get_positions(row_first, row_last);

            hilet x0 = ltor ? x_min : container_size.width() - x_max;
            hilet x3 = ltor ? x_max : container_size.width() - x_min;
            hilet y0 = container_size.height() - y_max;
            hilet y3 = container_size.height() - y_min;

            return {point2{x0, y0}, point2{x3, y3}};
        }

        [[nodiscard]] widget_baseline baseline(grid_layout const& rows) const noexcept
        {
            return rows.get_baseline(row_first, row_last);
        }
    };

    std::vector<cell_type> _cells;

    grid_layout _rows;
    grid_layout _columns;

    [[nodiscard]] bool
    address_in_use(std::size_t column_first, std::size_t row_first, std::size_t column_last, std::size_t row_last) const noexcept;

    /* Add a widget to the grid.
     */
    widget& add_widget(
        std::size_t column_first,
        std::size_t row_first,
        std::size_t column_last,
        std::size_t row_last,
        std::unique_ptr<widget> child_widget) noexcept;
};

}} // namespace hi::v1
