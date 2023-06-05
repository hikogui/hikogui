

#pragma once

#include "../geometry/module.hpp"

namespace hi { inline namespace v1 {

/** A cell in a grid.
 *
 * @ingroup layout
 */
class grid_cell {
public:
    constexpr grid_cell(grid_cell const&) = delete;
    constexpr grid_cell(grid_cell&& other) noexcept;
    constexpr grid_cell& operator=(grid_cell const&) = delete;
    constexpr grid_cell& operator=(grid_cell&& other) noexcept;
    [[nodiscard]] constexpr friend bool operator==(grid_cell const&, grid_cell const&) noexcept = default;

    constexpr ~grid_cell();
    constexpr grid_cell() noexcept = default;
    constexpr grid_cell(grid& grid) noexcept;

    constexpr grid_cell(grid_cell const& parent, uint8_t col_begin, uint8_t row_begin, uint8_t col_end, uint8_t row_end) noexcept
        :
        grid_cell(*parent._grid)
    {
        set_parent(parent);
        set_location(col_begin, row_begin, col_end, row_end);
    }

    constexpr grid_cell(grid_cell const& parent, uint8_t col_begin, uint8_t row_begin) noexcept : grid_cell(*parent._grid)
    {
        set_parent(parent);
        set_location(col_begin, row_begin);
    }

    constexpr grid_cell(grid_cell const& parent, std::string_view address) noexcept : grid_cell(*parent._grid)
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
    constexpr void set_parent(grid_cell const& parent) noexcept;

    /** Remove the parent for this child-cell.
     *
     * A cell without a parent would be the window-widget or overlay-widget.
     *
     * @note If there is no parent then the location must be (0, 0).
     */
    constexpr void unset_parent(grid_cell const& parent) noexcept;

    constexpr void set_priority(int8_t width_priority, int8_t height_priority) noexcept;

    constexpr void set_priority(int8_t priority = 0) noexcept
    {
        return set_priority(priority, priority);
    }

    constexpr void set_margin(hi::margins margin) noexcept;

    constexpr void set_margin(float margins) noexcept
    {
        return set_margin(hi::margins{margin});
    }

    constexpr void set_constraints(hi::extent2 minimum, hi::extent2 maximum, hi::extent2 wrap) noexcept;

    constexpr void set_constraints(hi::extent2 minimum, hi::extent2 maximum) noexcept
    {
        return set_constraints(minimum, maximum, minimum);
    }

    constexpr void set_constraints(hi::extent2 size) noexcept
    {
        return set_constraints(size);
    }

    constexpr void set_size(hi::extent2 size) noexcept;

    [[nodiscard]] constexpr aarectangle rectangle() const noexcept;

private:
    grid *_grid = nullptr;
    size_t _id = 0;
};

}} // namespace hi::v1
