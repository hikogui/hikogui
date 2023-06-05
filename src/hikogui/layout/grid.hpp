
/** @file layout/grid.hpp Types for handling layout of widgets.
 * @ingroup layout
 */

#pragma once

#include "spreadsheet_address.hpp"
#include "grid_axis.hpp"
#include "grid_cell.hpp"
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
    constexpr void constrain() noexcept
    {
        if (std::exchange(reconstrain, false)) {
            update_indices();
            calculate_row_col_count_and_margins();
            setup_row_col_tables();
            populate_row_col_tables();
            relayout = true;
        }
    }

    constexpr void layout() noexcept
    {
        constrain();
        if (std::exchange(relayout, false)) {
            layout_rows();
            layout_columns();
            position_children();
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

    [[nodiscard]] constexpr cell_type& operator[](size_t id) noexcept
    {
        hi_axiom_bounds(id, _cells);
        return _cells[id];
    }

    [[nodiscard]] constexpr cell_type const& operator[](size_t id) const noexcept
    {
        hi_axiom_bounds(id, _cells);
        return _cells[id];
    }

private:
    struct cell_type {
        int32_t parent = -1;
        uint32_t in_use : 1 = 0;
        uint32_t leaf : 1 = 0;
        uint32_t perminant_mark : 1 = 0;
        uint32_t temporary_mark : 1 = 0;
        uint32_t reserved : 28 = 0;

        /** Offset in the column table.
         */
        int32_t col_offset = 0;

        /** Offset in the row table.
         */
        int32_t row_offset = 0;

        int8_t col_begin = 0;
        int8_t col_end = 0;
        int8_t row_begin = 0;
        int8_t row_end = 0;

        /** The priority when the change the width compared to other cells in the row.
         */
        int8_t width_priority = 0;

        /** The priority when the change the height compared to other cells in the column.
         */
        int8_t height_priority = 0;

        /** The left-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        int8_t margin_left = 0;

        /** The bottom-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        int8_t margin_bottom = 0;

        /** The right-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        int8_t margin_right = 0;

        /** The top-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        int8_t margin_top = 0;

        /** The thinner width when the cell can wrap.
         */
        int32_t wrapped_width;

        /** The preferred width.
         */
        int32_t minimum_width;

        /** The maximum width.
         */
        int32_t maximum_width;

        /** The taller height when the cell can wrap.
         */
        int32_t wrapped_height;

        /** The minimum height.
         */
        int32_t minimum_height;

        /** The maximum height.
         */
        int32_t maximum_height;

        /** The left position of this cell relative to the parent.
         * @note This field is calculated.
         */
        int32_t left;

        /** The bottom position of this cell relative to the parent.
         * @note This field is calculated.
         */
        int32_t bottom;

        /** The width of this cell.
         * @note This field is calculated, except for the root grid.
         */
        int32_t width;

        /** The height of this cell.
         * @note This field is calculated, except for the root grid.
         */
        int32_t height;

        /** Number of columns based on the locations of this cell's children.
         * @note This field is calculated.
         */
        int8_t num_cols = 0;

        /** Number of rows based on the locations of this cell's children.
         * @note This field is calculated.
         */
        int8_t num_rows = 0;

        /** The left-margin (rtl: right-margin) calculated from children.
         * @note This field is calculated.
         */
        int8_t col_before_margin = 0;

        /** The top-margin calculated from children.
         * @note This field is calculated.
         */
        int8_t row_before_margin = 0;

        /** The right-margin (rtl: left-margin) calculated from children.
         * @note This field is calculated.
         */
        int8_t col_after_margin = 0;

        /** The bottom margin calculated from children.
         * @note This field is calculated.
         */
        int8_t row_after_margin = 0;
    };

    /** All cells, both used and part of the free-list.
     */
    std::vector<cell_type> _cells = {};

    /** Index to the first cell of the free-list.
     */
    int32_t _first_free = -1;

    /** A topologically sorted list of indices into the cell table.
     *
     * Entries are partitioned between leaves, non-leaves.
     * Entries are topologically ordered with the parents after children.
     */
    std::vector<int32_t> _indices = {};

    size_t _num_leaves = 0;
    size_t _num_grids = 0;

    /** An iterator denoting the end of the leaf-entries, and start of grid-entries.
     */
    decltype(_indices)::iterator _indices_split = {};

    /** Data for the combined rows of all grids.
     */
    grid_axis _rows = {};

    /** Data for the combined rows of all grids.
     */
    grid_axis _columns = {};

    /** Set to true when a value has changed that require the grids to be re-constrained.
     */
    bool reconstrain = true;

    /** Set to true when a value has changed that require the grids to be re-laid out.
     */
    bool relayout = true;

    constexpr void update_indices_visit(int32_t i, cell_type& n) noexcept
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

        // Put all the leaves at the start, so that we don't need to check
        // if a node is a leaf in the future.
        _indices_split = std::stable_partition(_indices.begin(), _indices.end(), [](hilet a) {
            return _cells[a].leaf;
        });

        _num_leaves = std::distance(_indices.begin(), _indices_split);
        _num_grids = std::distance(_indices_split, _indices.end());
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

    constexpr auto row_begin(cell_type const& cell) noexcept
    {
        return _rows.begin() + cell.row_offset;
    }

    constexpr auto col_end(cell_type const& cell) noexcept
    {
        return _cols.begin() + cell.col_offset;
    }

    constexpr auto row_pair(cell_type const& parent, cell_type const& child) noexcept
    {
        return std::make_tuple(row_begin(parent) + cell.row_begin, row_begin(parent) + cell.row_end);
    }

    constexpr auto col_pair(cell_type const& parent, cell_type const& child) noexcept
    {
        return std::make_tuple(col_begin(parent) + cell.col_begin, col_begin(parent) + cell.col_end);
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
};

}} // namespace hi::v1
