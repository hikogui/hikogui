
/** @file layout/super_grid.hpp Types for handling layout of widgets.
 * @ingroup layout
 */

#pragma once

#include "spreadsheet_address.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>

namespace hi { inline namespace v1 {

/** A cell in a grid.
 *
 * @ingroup layout
 */
class super_grid_cell {
public:
    constexpr super_grid_cell(super_grid_cell const&) = delete;
    constexpr super_grid_cell(super_grid_cell&& other) noexcept;
    constexpr super_grid_cell& operator=(super_grid_cell const&) = delete;
    constexpr super_grid_cell& operator=(super_grid_cell&& other) noexcept;
    [[nodiscard]] constexpr friend bool operator==(super_grid_cell const&, super_grid_cell const&) noexcept = default;

    constexpr ~super_grid_cell();
    constexpr super_grid_cell() noexcept = default;
    constexpr super_grid_cell(super_grid & grid) noexcept;

    constexpr super_grid_cell(super_grid_cell const &parent, uint8_t col_begin, uint8_t row_begin, uint8_t col_end, uint8_t row_end) noexcept :
        super_grid_cell(*parent._grid)
    {
        set_parent(parent);
        set_location(col_begin, row_begin, col_end, row_end);
    }

    constexpr super_grid_cell(super_grid_cell const &parent, uint8_t col_begin, uint8_t row_begin) noexcept :
        super_grid_cell(*parent._grid)
    {
        set_parent(parent);
        set_location(col_begin, row_begin);
    }

    constexpr super_grid_cell(super_grid_cell const &parent, std::string_view address) noexcept :
        super_grid_cell(*parent._grid)
    {
        set_parent(parent);
        set_location(address);
    }

    /** Check if this cell has a location.
     */
    [[nodiscard]] constexpr bool empty() const noexcept;

    /** Clear the cell.
     *
     * This will remove the parent, location, constraints and layout.
     */
    constexpr void clear() noexcept;

    /** Set the location of a cell.
     *
     * @param col_begin The first column in a col-span.
     * @param row_begin The first row in a row-span.
     * @param col_end One beyond the last column in a col-span.
     * @param row_end One beyond the last row in a row-span.
     */
    constexpr void set_location(uint8_t col_begin, uint8_t row_begin, uint8_t col_end, uint8_t row_end) noexcept;

    /** Set the location of a cell.
     *
     * @note This will create a col-span and row-span of 1.
     * @param col_begin The column in the grid.
     * @param row_begin The row in the grid.
     */
    constexpr void set_location(uint8_t col, uint8_t row) noexcept
    {
        return set_location(col, col + 1, row, row + 1);
    }

    /** Set the location and span of a cell based on the spreadsheet address.
     *
     * @param address A spreadsheet like address.
     */
    constexpr void set_location(std::string_view address) noexcept
    {
        [ col_begin, row_begin, col_end, row_end ] = parse_spreadsheet_range(address);
        return set_location(col_begin, row_begin, col_end, row_end)
    }

    /** Set the location to the origin of the grid.
     */
    constexpr void set_location() noexcept
    {
        return set_location(0, 0);
    }

    /** Set the parent for this child-cell.
     *
     * @param parent A parent cell.
     */
    constexpr void set_parent(super_grid_cell const& parent) noexcept;

    /** Remove the parent for this child-cell.
     *
     * A cell without a parent would be the window-widget or overlay-widget.
     *
     * @note If there is no parent then the location must be (0, 0).
     */
    constexpr void unset_parent(super_grid_cell const& parent) noexcept;


private:
    super_grid *_grid = nullptr;
    size_t _id = 0;
};

/**
 *
 *
 * The layout-algorithm:
 *
 */
class super_grid {
public:
    super_grid(super_grid const&) = delete;
    super_grid(super_grid&&) = delete;
    super_grid& operator=(super_grid const&) = delete;
    super_grid& operator=(super_grid&&) = delete;

    constexpr super_grid() noexcept = default;

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
        update_indices();
        update_grid_margins();
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
        int in_use : 1 = 0;
        int leaf : 1 = 0;
        int perminant_mark : 1 = 0;
        int temporary_mark : 1 = 0;
        int reserved : 28 = 0;

        /** Offset in the column table.
         */
        int col_offset = 0;

        /** Offset in the row table.
         */

        int row_offset = 0;

        uint8_t col_begin = 0;
        uint8_t col_end = 0;
        uint8_t row_begin = 0;
        uint8_t row_end = 0;

        /** The priority when the change the width compared to other cells in the row.
         */
        uint8_t width_priority = 0;

        /** The priority when the change the height compared to other cells in the column.
         */
        uint8_t height_priority = 0;

        /** The left-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        uint8_t margin_left = 0;

        /** The bottom-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        uint8_t margin_bottom = 0;

        /** The right-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        uint8_t margin_right = 0;

        /** The top-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        uint8_t margin_top = 0;

        /** The thinner width when the cell can wrap.
         */
        int wrapped_width;

        /** The preferred width.
         */
        int preferred_width;

        /** The maximum width.
         */
        int maximum_width;

        /** The taller height when the cell can wrap.
         */
        int wrapped_height;

        /** The minimum height.
         */
        int minimum_height;

        /** The maximum height.
         */
        int maximum_height;

        /** Number of columns based on the locations of this cell's children.
         * @note This field is calculated.
         */
        uint8_t num_cols = 0;

        /** Number of rows based on the locations of this cell's children.
         * @note This field is calculated.
         */
        uint8_t num_rows = 0;

        /** The left-margin (rtl: right-margin) calculated from children.
         * @note This field is calculated.
         */
        uint8_t col_before_margin = 0;

        /** The top-margin calculated from children.
         * @note This field is calculated.
         */
        uint8_t row_before_margin = 0;

        /** The right-margin (rtl: left-margin) calculated from children.
         * @note This field is calculated.
         */
        uint8_t col_after_margin = 0;

        /** The bottom margin calculated from children.
         * @note This field is calculated.
         */
        uint8_t row_after_margin = 0;

        [[nodiscard]] constexpr uint8_t col_span() const noexcept
        {
            return col_end - col_begin;
        }

        [[nodiscard]] constexpr uint8_t row_span() const noexcept
        {
            return row_end - row_begin;
        }
    };

    struct row_type {
        int minimum = 0;
        int maximum = std::numeric_limits<int>::max();
        uint8_t priority = 0;
        uint8_t before_margin = 0;
        uint8_t after_margin = 0;
    };

    struct column_type {
        int wrapped = 0;
        int preferred = 0;
        utin32_t maximum = std::numeric_limits<int>::max();
        uint8_t priority = 0;
        uint8_t before_margin = 0;
        uint8_t after_margin = 0;
    };

    /** All cells, both used and part of the free-list.
     */
    std::vector<cell_type> _cells = {};

    /** A topologically sorted list of indices into the cell table.
     *
     * Entries are partitioned between leaves, non-leaves.
     * Entries are topologically ordered with the parents after children.
     */
    std::vector<int32_t> _indices = {};

    /** Data for the combined rows of all grids.
     */
    std::vector<row_type> _rows = {};

    /** Data for the combined rows of all grids.
     */
    std::vector<row_type> _columns = {};

    size_t _num_leaves = 0;
    size_t _num_grids = 0;

    /** An iterator denoting the end of the leaf-entries, and start of grid-entries.
     */
    decltype(_indices)::iterator _indices_split = {};

    /** Index to the first cell of the free-list.
     */
    int32_t _first_free = -1;

    template<typename It, std::sentinal_for<It> ItEnd, std::invocable<decltype(*std::declval_t<It>())> F>
    constexpr static int extent(It first, ItEnd last, F const& f) noexcept
    {
        hi_axiom(first != last);
        auto r = f(*it);
        for (auto it = first + 1; it != last; ++it) {
            r += it->before_margin;
            r += f(*it);
        }
        return r;
    }

    template<typename It, std::sentinal_for<It> ItEnd, std::invocable<decltype(*std::declval_t<It>())> F>
    constexpr static int expand(It first, ItEnd last, int target_size, F const& f) noexcept
    {

    }

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
        _cols.clear();
        _rows.clear();
        _cols.resize(num_cols);
        _rows.resize(num_rows);
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
        for (hilet& cell : _cells) {
            if (cell.parent == -1 or not cell.in_use) {
                continue;
            }

            hilet& parent = _cells[cell.parent];

            auto [first_row, last_row] = row_pair(parent, cell);
            inplace_max(first_row->before_margin, cell.row_before_margin);
            inplace_max((last_row - 1)->after_margin, cell.row_after_margin);
            if (std::distance(first_row, last_row) == 1) {
                inplace_max(first_row->minimum, cell.minimum_height);
                inplace_min(first_row->maximum, cell.maximum_height);
            }

            auto [first_col, last_col] = col_pair(parent, cell);
            inplace_max(first_col->before_margin, cell.col_before_margin);
            inplace_max((last_col - 1)->after_margin, cell.col_after_margin);
            if (std::distance(first_col, last_col) == 1) {
                inplace_max(first_col->preferred, cell.preferred_width);
                inplace_min(first_col->maximum, cell.maximum_width);
            }
        }

        // Merge margins in rows and columns.
        for (auto i = 1; i != _rows.size(); ++i) {
            hilet margin = std::max(_rows[i - 1].after_margin, _rows[i].before_margin);
            _rows[i - 1].after_margin = _rows[i].before_margin = margin;
        }
        for (auto i = 1; i != _cols.size(); ++i) {
            hilet margin = std::max(_cols[i - 1].after_margin, _cols[i].before_margin);
            _cols[i - 1].after_margin = _cols[i].before_margin = margin;
        }

        // For col-span > 1 and/or row-span > 1 we need to expand the column
        // and row based on priority.
        for (hilet& cell : _cells) {
            if (cell.parent == -1 or not cell.in_use) {
                continue;
            }

            hilet& parent = _cells[cell.parent];

            auto [first_row, last_row] = row_pair(parent, cell);
            if (std::distance(first_row, last_row) > 1) {
                expand_minimum(cell.minimum_height, &first_row, &last_row);
                shrink_maximum(cell.maximum_height, &first_row, &last_row);
            }

            auto [first_col, last_col] = col_pair(parent, cell);
            if (std::distance(first_col, last_col) > 1) {
                expand_preferred(cell.preferred_width, &first_col, &last_col);
                shrink_maximum(cell.maximum_width, &first_col, &last_col);
            }
        }
    }

    /** Update the margins of each grid based on the contained cells.
     */
    constexpr void update_grid_margins() noexcept
    {
        calculate_row_col_count_and_margins();
        setup_row_col_tables();
        populate_row_col_tables();
    }
};

constexpr ~super_grid_cell::super_grid_cell()
{
    if (_grid) {
        _grid->remove_cell(_id);
    }
}

constexpr super_grid_cell::super_grid_cell(super_grid_cell&& other) noexcept : _grid(other._grid), _id(other._id)
{
    other._grid = nullptr;
}

constexpr super_grid_cell& super_grid_cell::operator=(super_grid_cell&& other) noexcept
{
    if (_grid) {
        _grid->remove_cell(_id);
    }
    _grid = std::exchange(other._grid, nullptr);
    _id = other._id;
}

constexpr super_grid_cell::super_grid_cell(super_grid const& grid) noexcept : _grid(std::addressof(grid)), _id(grid.add_cell()) {}

[[nodiscard]] constexpr bool super_grid_cell::empty() const noexcept
{
    return _grid->at(_id).in_use == 0;
}

constexpr void super_grid_cell::clear() noexcept
{
    _grid->at(_id).in_use = 0;
}

constexpr void super_grid_cell::set_location(uint8_t col_begin, uint8_t row_begin, uint8_t col_end, uint8_t row_end) noexcept
{
    hi_axiom(col_begin < std::numeric_limits<uint8_t>::max());
    hi_axiom(row_begin < std::numeric_limits<uint8_t>::max());
    hi_axiom(col_begin < col_end);
    hi_axiom(row_begin < row_end);

    _grid->at(_id).col_begin = col_begin;
    _grid->at(_id).row_begin = row_begin;
    _grid->at(_id).col_end = col_end;
    _grid->at(_id).row_end = row_end;
}

constexpr void super_grid_cell::set_parent(super_grid_cell const& parent) noexcept
{
    _grid->at(_id).parent = parent._id;
}

constexpr void super_grid_cell::unset_parent() noexcept
{
    (*_grid)[_id].parent == -1;
}

}} // namespace hi::v1
