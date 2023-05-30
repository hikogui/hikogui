

#pragma once

#include "spreadsheet_address.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>

namespace hi { inline namespace v1 {

class super_grid_cell {
public:
    constexpr super_grid_cell(super_grid_cell const&) = delete;
    constexpr super_grid_cell(super_grid_cell&& other) noexcept;
    constexpr super_grid_cell& operator=(super_grid_cell const&) = delete;
    constexpr super_grid_cell& operator=(super_grid_cell&& other) noexcept;
    [[nodiscard]] constexpr friend bool operator==(super_grid_cell const&, super_grid_cell const&) noexcept = default;

    constexpr ~super_grid_cell();
    constexpr super_grid_cell() noexcept = default;
    constexpr super_grid_cell(super_grid const& grid) noexcept;

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

    constexpr void remove_cell(size_t id) noexcept
    {
        _entries[id].make_free(std::exchange(_first_free, id));
    }

    [[nodiscard]] constexpr size_t add_cell() noexcept
    {
        hilet id = [&] {
            if (_first_free != std::numeric_limits<size_t>::max()) {
                hilet next_free = _entries[_first_free].parent;
                return std::exchange(_first_free, next_free);

            } else {
                hilet id = _entries.size();
                _entries.emplace_back();
                return id;
            }
        }();

        _entries[id].parent = std::numeric_limits<size_t>::max();
        return id;
    }

    [[nodiscard]] constexpr entry_type& operator[](size_t id) noexcept
    {
        hi_axiom_bounds(id, _entries);
        return _entries[id];
    }

    [[nodiscard]] constexpr entry_type const& operator[](size_t id) const noexcept
    {
        hi_axiom_bounds(id, _entries);
        return _entries[id];
    }

private:
    class entry_type {
    public:
        constexpr entry_type() noexcept = default;
        constexpr entry_type(entry_type const&) noexcept = default;
        constexpr entry_type(entry_type&&) noexcept = default;
        constexpr entry_type& operator=(entry_type const&) noexcept = default;
        constexpr entry_type& operator=(entry_type&&) noexcept = default;

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return col_end == 0;
        }

        constexpr void clear() noexcept
        {
            _parent = std::numeric_limits<size_t>::max();
            _col_begin = 0;
            _col_end = 0;
            _row_begin = 0;
            _row_end = 0;
        }

        constexpr void make_free(size_t next_free) noexcept
        {
            clear();
            _parent = next_free;
        }

        constexpr void set_location(uint8_t col_begin, uint8_t row_begin, uint8_t col_end, uint8_t row_end) noexcept
        {
            hi_axiom(col_begin < std::numeric_limits<uint8_t>::max());
            hi_axiom(row_begin < std::numeric_limits<uint8_t>::max());
            hi_axiom(col_begin < col_end);
            hi_axiom(row_begin < row_end);
            _col_begin = col_begin;
            _row_begin = row_begin;
            _col_end = col_end;
            _row_end = row_end;
        }

        constexpr void set_parent(size_t parent) noexcept
        {
            hi_axiom(parent != std::numeric_limits<size_t>::max()) l _parent = parent;
        }

        constexpr void unset_parent() noexcept
        {
            _parent = std::numeric_limits<size_t>::max()
        }

    private:
        size_t _parent = std::numeric_limits<size_t>::max();
        uint8_t _col_begin = 0;
        uint8_t _col_end = 0;
        uint8_t _row_begin = 0;
        uint8_t _row_end = 0;

        /** Number of columns based on the locations of this cell's children.
         * @note This field is calculated.
         */
        uint8_t _num_cols = 0;

        /** Number of rows based on the locations of this cell's children.
         * @note This field is calculated.
         */
        uint8_t _num_rows = 0;

        /** The priority when the change the width compared to other cells in the row.
         */
        uint8_t _width_priority;

        /** The priority when the change the height compared to other cells in the column.
         */
        uint8_t _height_priority;

        /** The left-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        uint8_t _margin_left = 0;

        /** The bottom-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        uint8_t _margin_bottom = 0;

        /** The right-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        uint8_t _margin_right = 0;

        /** The top-margin for this cell.
         * @note For non-leaf cells this is calculated.
         */
        uint8_t _margin_top = 0;

        /** The thinner width when the cell can wrap.
         */
        uint32_t _wrapped_width;

        /** The preferred width.
         */
        uint32_t _preferred_width;

        /** The maximum width.
         */
        uint32_t _maximum_width;

        /** The taller height when the cell can wrap.
         */
        uint32_t _wrapped_height;

        /** The preferred height.
         */
        uint32_t _preferred_height;

        /** The maximum height.
         */
        uint32_t _maximum_height;
    };

    std::vector<entry_type> _entries = {};
    size_t _first_free = std::numeric_limits<size_t>::max();
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
    return _grid[_id].empty();
}

constexpr void super_grid_cell::clear() noexcept
{
    return _grid[_id].clear();
}

constexpr void super_grid_cell::set_location(uint8_t col_begin, uint8_t row_begin, uint8_t col_end, uint8_t row_end) noexcept
{
    return _grid[_id].set_location(col_begin, row_begin, col_end, row_end);
}

constexpr void super_grid_cell::set_parent(super_grid_cell const& parent) noexcept
{
    return _grid[_id].set_parent(parent._id);
}

constexpr void super_grid_cell::unset_parent() noexcept
{
    _grid[_id].set_location(0, 0, 1, 1);
    return _grid[_id].unset_parent();
}

}} // namespace hi::v1
