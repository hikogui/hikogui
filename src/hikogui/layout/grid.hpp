
/** @file layout/grid.hpp Types for handling layout of widgets.
 * @ingroup layout
 */

#pragma once

#include "spreadsheet_address.hpp"
#include "grid_axis.hpp"
#include "grid_cell.hpp"
#include "grid_state.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>

namespace hi { inline namespace v1 {

/**
 *
 *
 * The layout-algorithm:
 *
 */
class grid {
public:
    grid(grid const&) = delete;
    grid(grid&&) = delete;
    grid& operator=(grid const&) = delete;
    grid& operator=(grid&&) = delete;

    constexpr grid() noexcept = default;

    /** Calculate the constraints for the grid.
     *
     * The constrain-algorithm:
     *  1. Assign priorities to each row and column.
     *  2. Assign margins to each row and column.
     *  3. Calculate the preferred-height of each row.
     *     - First handle row-span = 1
     *     - Next handle row-span > 1, and extent rows based on the priority assigned
     *       to each row.
     *  4. Calculate the preferred-width of each column.
     *     - First handle col-span = 1
     *     - Next handle col-span > 1, and extent columns based on the priority
     *       assigned to each column.
     *  5. Calculate the minimum-width of each column; select preferred-width or
     *     wrapped-width depending if the cell's wrapped-height fits into the
     *     preferred-height of the row it is in.
     */

    constexpr void update() noexcept
    {
        if (state != grid_state::done) {
            [[unlikely]] _update();
            state = 0;
        }
    }

    constexpr void remove_cell(size_t id) noexcept
    {
        _cells[id].make_free(std::exchange(_first_free, id));
    }

    [[nodiscard]] constexpr size_t add_cell() noexcept
    {
        hilet id = [&] {
            if (_first_free != std::numeric_limits<size_t>::max()) {
                hilet next_free = _cells[_first_free].parent;
                return std::exchange(_first_free, next_free);

            } else {
                hilet id = _cells.size();
                _cells.emplace_back();
                return id;
            }
        }();

        _cells[id].parent = std::numeric_limits<size_t>::max();
        return id;
    }

    [[nodiscard]] constexpr detail::grid_cell_data& operator[](size_t id) noexcept
    {
        hi_axiom_bounds(id, _cells);
        return _cells[id];
    }

    [[nodiscard]] constexpr detail::grid_cell_data const& operator[](size_t id) const noexcept
    {
        hi_axiom_bounds(id, _cells);
        return _cells[id];
    }

private:
    /** All cells, both used and part of the free-list.
     */
    std::vector<detail::grid_cell_data> _cells = {};

    /** Index to the first cell of the free-list.
     */
    int32_t _first_free = -1;

    /** A topologically sorted list of indices into the cell table.
     *
     * Entries are partitioned between leaves, non-leaves.
     * Entries are topologically ordered with the parents after children.
     */
    std::vector<int32_t> _indices = {};

    /** An iterator pointing to the first entry that is not a leaf.
     */
    decltype(_indices)::iterator _grid_begin = {};

    /** An iterator pointing to the first entry that is a root.
     */
    decltype(_indices)::iterator _root_begin = {};

    /** Data for the combined rows of all grids.
     */
    grid_axis _rows = {};

    /** Data for the combined rows of all grids.
     */
    grid_axis _columns = {};

    /** The state determines what needs to be updated.
     */
    grid_state _state = grid_state::need_constrain;

    constexpr auto row_begin(detail::grid_cell_data const& cell) noexcept
    {
        return _rows.begin() + cell.row_offset;
    }

    constexpr auto col_end(detail::grid_cell_data const& cell) noexcept
    {
        return _cols.begin() + cell.col_offset;
    }

    constexpr auto row_pair(detail::grid_cell_data const& parent, detail::grid_cell_data const& child) noexcept
    {
        return std::make_tuple(row_begin(parent) + cell.row_begin, row_begin(parent) + cell.row_end);
    }

    constexpr auto col_pair(detail::grid_cell_data const& parent, detail::grid_cell_data const& child) noexcept
    {
        return std::make_tuple(col_begin(parent) + cell.col_begin, col_begin(parent) + cell.col_end);
    }

    constexpr void update_indices_visit(int32_t i, detail::grid_cell_data& n) noexcept
    {
        hi_axiom(n.in_use);

        if (n.permanent_mark) {
            return;
        }
        if (n.temporary_mark) {
            hi_no_default("Loop found in super-grid.");
        }

        n.temporary_mark = 1;
        if (n.parent != -1) {
            auto& p = _cells[n.parent];
            p.leaf = 0;
            make_tree_visit(n.parent, p);
        }

        n.temporary_mark = 0;
        n.permanent_mark = 1;
        _indices.push_back(i);
    }

    /** Make a topological sorted list of cells.
     */
    constexpr void update_indices() noexcept
    {
        // Calculate how many children each node has and if the node is a leaf.
        // This also works on entries that are on the free-list.
        for (auto& cell : _cells) {
            cell.leaf = 1;
            cell.perminant_mark = 0;
            cell.temporary_mark = 0;
        }

        // depth-first topological sort.
        _indices.clear();
        for (int32_t i = 0; i != std::ssize(_cells); ++i) {
            auto& n = _cells[i];
            if (n.in_use) {
                update_indices_visit(i, n);
            }
        }

        // The ordering is parents first, children last, reverse this.
        std::reverse(_indices.begin(), _indices.end());

        // Put all the leaves at the start
        _grid_begin = std::stable_partition(_indices.begin(), _indices.end(), [](hilet a) {
            return _cells[a].leaf;
        });

        // Put all the root entries at the end.
        _root_begin = std::stable_partition(_grid_begin, _indices.end(), [](hilet a) {
            return _cells[a].parent != -1;
        }
    }

    constexpr void calculate_row_col_count_and_margins() noexcept
    {
        // This also works on entries that are on the free-list.
        for (auto& cell : _cells) {
            cell.num_cols = 0;
            cell.num_rows = 0;
            cell.row_before_margin = cell.top_margin;
            cell.col_before_margin = _left_to_right ? cell.left_margin : cell.right_margin;
            cell.row_after_margin = cell.bottom_margin;
            cell.col_after_margin = _left_to_right ? cell.right_margin : cell.left_margin;
        }

        // This also works on entries that are on the free-list.
        for (hilet& cell : _cells) {
            if (cell.parent == -1) {
                continue;
            }

            inplace_max(_cells[cell.parent].num_cols, cell.col_end);
            inplace_max(_cells[cell.parent].num_rows, cell.row_end);
        }

        // Calculate the total margin of each grid.
        // This is done in topological order, so that grids inside grids get
        // the correct margins.
        for (hilet i : _indices) {
            hilet& cell : _cells[i];

            auto& parent = _cells[cell.parent];
            if (cell.col_begin == 0) {
                inplace_max(parent.col_before_margin, cell.col_before_margin);
            }
            if (cell.row_begin == 0) {
                inplace_max(parent.row_before_margin, cell.row_before_margin);
            }
            if (cell.col_end == parent.num_cols) {
                inplace_max(parent.col_after_margin, cell.col_after_margin);
            }
            if (cell.row_end == parent.num_row) {
                inplace_max(parent.row_after_margin, cell.row_after_margin);
            }
        }
    }

    constexpr void setup_row_col_tables() noexcept
    {
        auto num_rows = 0;
        auto num_cols = 0;
        for (auto it = _indices_split; it != _indices.end(); ++it) {
            auto& cell = _cells[*it];
            hi_axiom(cell.num_cols != 0);
            hi_axiom(cell.num_rows != 0);

            cell.col_offset = num_cols;
            cell.row_offset = num_rows;
            num_cols += cell.num_cols;
            num_rows += cell.num_rows;
        }
        _cols.clear(num_cols);
        _rows.clear(num_rows);
    }

    constexpr void populate_row_col_tables() noexcept
    {
        // First step is filling in the row and column tables based on
        // data from each cell. We are only filling in the minimums and maximums
        // of single-span cells, as it allows multi-span to more properly scale
        // the rows and columns.
        for (hilet& cell : _cells) {
            if (cell.parent != -1 and cell.in_use) {
                hilet& parent = _cells[cell.parent];

                hilet[first_row, last_row] = row_pair(parent, cell);
                set_priority(first_row, last_row, cell.row_priority);
                set_margins(first_row, last_row, cell.row_before_margin, cell.row_after_margin);

                if (std::distance(first_row, last_row) == 1) {
                    set_minimum(first_row, last_row, cell.minimum_height);
                    set_preferred(first_row, last_row, cell.minimum_height);
                    set_maximum(first_row, last_row, cell.maximum_height);
                }

                hilet[first_col, last_col] = col_pair(parent, cell);
                set_priority(first_col, last_col, cell.row_priority);
                set_margins(first_col, last_col, cell.col_before_margin, cell.col_after_margin);

                if (std::distance(first_col, last_col) == 1) {
                    // The minimum width is determined after knowing all row heights.
                    set_preferred(first_col, last_col, cell.minimum_width);
                    set_maximum(first_col, last_col, cell.maximum_width);
                }
            }
        }

        // Now that we know the proper minimum and maximum sizes of the rows
        // and columns. We can scale them to fit multi-span cells.
        for (hilet& cell : _cells) {
            if (cell.parent != -1 and cell.in_use) {
                hilet& parent = _cells[cell.parent];

                hilet[first_row, last_row] = row_pair(parent, cell);
                if (std::distance(first_row, last_row) > 1) {
                    set_minimum(first_row, last_row, cell.minimum_height);
                    set_preferred(first_row, last_row, cell.minimum_height);
                    set_maximum(first_row, last_row, cell.maximum_height);
                }

                hilet[first_col, last_col] = col_pair(parent, cell);
                if (std::distance(first_col, last_col) > 1) {
                    // The minimum width is determined after knowing all row heights.
                    set_preferred(first_col, last_col, cell.minimum_width);
                    set_maximum(first_col, last_col, cell.maximum_width);
                }
            }
        }

        // Now that we know the minimum-height of each row, we can see if it is
        // possible to wrap cells to become less wide while keeping inside the
        // height requirements.
        for (hilet& cell : _cells) {
            if (cell.parent != -1 and cell.in_use) {
                hilet[first_row, last_row] = row_pair(parent, cell);
                hilet minimum_height = get_minimum(first_row, last_row);

                hilet[first_col, last_col] = col_pair(parent, cell);
                hilet minimum_width = cell.wrapped_height <= minimum_height ? cell.wrapped_width : cell.minimum_width);
                set_minimum(first_col, last_col, minimum_width);
            }
        }
    }

    constexpr void constrain() noexcept
    {
        update_indices();
        calculate_row_col_count_and_margins();
        setup_row_col_tables();
        populate_row_col_tables();
    }

    constexpr void layout() noexcept
    {
        // By iterating in reverse we start with the root grids, for which
        // the width and height are known.
        for (auto it = _indices.rbegin(); it != _indices.rend(); ++it) {
            auto& cell = _cells[*it];

            if (cell.parent != -1) {
                // If this cell has a parent, determine the width and height of
                // this cell.
                hilet &parent = _cells[cell.parent];
                hilet row_first = row_begin(parent) + cell.row_begin;
                hilet row_last = row_begin(parent) + cell.row_end;
                hilet col_first = col_begin(parent) + cell.col_begin;
                hilet col_last = col_begin(parent) + cell.col_end;
                cell.height = get_size(row_first, row_last);
                cell.width = get_size(col_first, col_last);
            }

            if (not cell.leaf) {
                // For each grid calculate the sizes and positions for rows
                // and columns
                hilet [row_first, row_last] = row_pair(grid);
                update_size(row_first, row_last, grid.height);
                update_position(row_first, row_last);

                hilet[col_first, col_last] = col_pair(grid);
                update_size(col_first, col_last, grid.width);
                update_position(col_first, col_last);
            }
        }
    }

    hi_no_inline constexpr void _update() noexcept
    {
        if (to_bool(state & grid_state::need_constrain)) {
            constraints();
        }
        layout();
    }
};

}} // namespace hi::v1
