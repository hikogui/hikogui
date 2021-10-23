// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "grid_delegate.hpp"
#include "../geometry/spreadsheet_address.hpp"
#include "../flow_layout.hpp"
#include "../weak_or_unique_ptr.hpp"
#include <memory>

namespace tt {

/** A GUI widget that lays out child-widgets in a grid with variable sized cells.
 *
 * The grid widget lays out child widgets in a grid pattern. Each child widget
 * occupies a single cell, which belongs into a single column and a single row.
 *
 * Columns are laid out from left to right, and rows from top to bottom. The row
 * and columns number may be specified as integers, or using an spreadsheet-like
 * cell-address:
 *  - `grid_widget::make_widget<T>(size_t column_nr, size_t row_nr, ...)`
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
    using delegate_type = grid_delegate;

    ~grid_widget();

    /** Constructs an empty grid widget.
     *
     * @param window The window.
     * @param parent The parent widget.
     * @param delegate An optional delegate can be used to populate the grid widget
     *                 during initialization.
     */
    grid_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate = {}) noexcept;

    /** Add a widget directly to this grid-widget.
     *
     * @tparam Widget The type of the widget to be constructed.
     * @param column_nr The zero-based index from left-to-right.
     * @param row_nr The zero-based index from top-to-bottom.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget &make_widget(size_t column_nr, size_t row_nr, Args &&...args)
    {
        auto tmp = std::make_unique<Widget>(window, this, std::forward<Args>(args)...);
        return static_cast<Widget &>(add_widget(column_nr, row_nr, std::move(tmp)));
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
    Widget &make_widget(std::string_view address, Args &&...args)
    {
        ttlet[column_nr, row_nr] = parse_spreadsheet_address(address);
        return make_widget<Widget>(column_nr, row_nr, std::forward<Args>(args)...);
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        for (ttlet &cell: _cells) {
            co_yield cell.widget.get();
        }
    }

    [[nodiscard]] float margin() const noexcept override;
    widget_constraints const &set_constraints() noexcept override;
    void set_layout(widget_layout const &context) noexcept override;
    void draw(draw_context const &context) noexcept override;
    /// @endprivatesection
private:
    struct cell {
        size_t column_nr;
        size_t row_nr;
        std::unique_ptr<tt::widget> widget;

        cell(size_t column_nr, size_t row_nr, std::unique_ptr<tt::widget> widget) noexcept :
            column_nr(column_nr), row_nr(row_nr), widget(std::move(widget))
        {
        }

        [[nodiscard]] aarectangle
        rectangle(flow_layout const &columns, flow_layout const &rows, float container_height) const noexcept
        {
            ttlet[x, width] = columns.get_offset_and_size(column_nr);
            ttlet[y, height] = rows.get_offset_and_size(row_nr);

            return {x, container_height - y - height, width, height};
        };
    };

    std::vector<cell> _cells;

    flow_layout _rows;
    flow_layout _columns;

    std::weak_ptr<delegate_type> _delegate;

    [[nodiscard]] static std::pair<size_t, size_t> calculate_grid_size(std::vector<cell> const &cells) noexcept;
    [[nodiscard]] static std::tuple<extent2, extent2, extent2>
    calculate_size(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept;
    [[nodiscard]] bool address_in_use(size_t column_nr, size_t row_nr) const noexcept;

    /* Add a widget to the grid.
     */
    widget &add_widget(size_t column_nr, size_t row_nr, std::unique_ptr<widget> child_widget) noexcept;
};

} // namespace tt
