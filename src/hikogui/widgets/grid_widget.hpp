// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/grid_widget.hpp Defines grid_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../layout/grid_layout.hpp"
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
     * @param first_column The first (left-most) column to place the widget in.
     * @param first_row The first (top-most) row to place the widget in.
     * @param last_column One beyond the last column to place the widget in.
     * @param last_row One beyond the last row to place the widget in.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget&
    make_widget(std::size_t first_column, std::size_t first_row, std::size_t last_column, std::size_t last_row, Args&&...args)
    {
        hi_axiom(first_column < last_column);
        hi_axiom(first_row < last_row);
        auto tmp = std::make_unique<Widget>(this, std::forward<Args>(args)...);
        return static_cast<Widget&>(add_widget(first_column, first_row, last_column, last_row, std::move(tmp)));
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
        return make_widget<Widget>(column, row, column + 1, row + 1, std::forward<Args>(args)...);
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
    [[nodiscard]] generator<widget const &> children(bool include_invisible) const noexcept override
    {
        for (hilet& cell : _grid) {
            co_yield *cell.value;
        }
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override;
    /// @endprivatesection
private:
    grid_layout<std::unique_ptr<widget>> _grid;

    /* Add a widget to the grid.
     */
    widget& add_widget(
        std::size_t first_column,
        std::size_t first_row,
        std::size_t last_column,
        std::size_t last_row,
        std::unique_ptr<widget> child_widget) noexcept;
};

}} // namespace hi::v1
