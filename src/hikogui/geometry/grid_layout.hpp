// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/box_constraints.hpp"
#include "spreadsheet_address.hpp"
#include "../cast.hpp"
#include <cstdint>
#include <numeric>
#include <vector>
#include <algorithm>
#include <utility>

namespace hi { inline namespace v1 {

template<typename T>
class grid_layout {
public:
    using value_type = T;

    struct cell_type {
        size_t first_column;
        size_t last_column;
        size_t first_row;
        size_t last_row;
        box_constraints constraints;
        value_type value;

        constexpr size_t colspan() const noexcept
        {
            return last_column - first_column;
        }

        constexpr size_t rowspan() const noexcept
        {
            return last_row - first_row;
        }
    };

    using cell_vector = std::vector<cell_type>;
    using iterator = cell_vector::iterator;
    using const_iterator = cell_vector::const_iterator;
    using reference = cell_vector::reference;

    ~grid_layout() = default;
    constexpr grid_layout() noexcept = default;
    grid_layout(grid_layout const&) noexcept = default;
    grid_layout(grid_layout&&) noexcept = default;
    grid_layout& operator=(grid_layout const&) noexcept = default;
    grid_layout& operator=(grid_layout&&) noexcept = default;
    [[nodiscard]] friend bool operator==(grid_layout const&, grid_layout const&) noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _cells.empty();
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _cells.size();
    }

    [[nodiscard]] constexpr size_t num_columns() const noexcept
    {
        return _num_columns;
    }

    [[nodiscard]] constexpr size_t num_rows() const noexcept
    {
        return _num_rows;
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return _cells.cbegin();
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return _cells.cbegin();
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param first_column The first column of the cell-span.
     * @param last_column One beyond the last column of the cell-span.
     * @param first_row The first row of the cell-span.
     * @param last_row One beyond the last row of the cell-span.
     * @retval true If the given cell-span overlaps with an already existing cell.
     */
    [[nodiscard]] bool cell_in_use(size_t first_column, size_t last_column, size_t first_row, size_t last_row) noexcept
    {
        // At least one cell must be in the range.
        hi_axiom(first_column < last_column);
        hi_axiom(first_row < last_row);

        for (hilet& cell : _cells) {
            if (first_column >= cell.last_column) {
                continue;
            }
            if (last_column <= cell.first_column) {
                continue;
            }
            if (first_row >= cell.last_row) {
                continue;
            }
            if (last_row <= cell.first_row) {
                continue;
            }
            return true;
        }
        return false;
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param address The spreadsheet address of the cell-span.
     * @retval true If the given cell-span overlaps with an already existing cell.
     */
    [[nodiscard]] bool cell_in_use(spreadsheet_address address)
    {
        hilet[first_column, first_row, last_column, last_row] = parse_spreadsheet_range(address);
        return cell_in_use(first_column, last_column, first_row, last_row);
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param first_column The first column of the cell-span.
     * @param last_column One beyond the last column of the cell-span.
     * @param first_row The first row of the cell-span.
     * @param last_row One beyond the last row of the cell-span.
     * @param value The value to be copied or moved into the cell.
     * @return A reference to the created cell.
     */
    template<forward_of<value_type> Value>
    reference add_cell(size_t first_column, size_t last_column, size_t first_row, size_t last_row, Value&& value) noexcept
    {
        // At least one cell must be in the range.
        hi_axiom(first_column < last_column);
        hi_axiom(first_row < last_row);
        hi_axiom(not cell_in_use(first_column, last_column, first_row, last_row));
        _cells.emplace_back(first_column, last_column, first_row, last_row, std::forward<Value>(value));
        update_after_insert_or_delete();
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param address The spreadsheet address of the cell-span.
     * @param value The value to be copied or moved into the cell.
     * @return A reference to the created cell.
     */
    template<forward_of<value_type> Value>
    reference add_cell(spreadsheet_address address, Value&& value)
    {
        hilet[first_column, first_row, last_column, last_row] = parse_spreadsheet_range(address);
        if (first_column >= last_column or first_row >= last_row) {
            throw parse_error("spreadsheet range must contain at least one cell.");
        }

        return add_cell(first_column, last_column, first_row, last_row, std::forward<Value>(value));
    }

    [[nodiscard]] box_constraints get_constraints() noexcept
    {
        auto r = box_constraints{};

        auto heights = get_minimum_heights();
        auto widths = get_widths(heights);
        r.top_margin = saturate_cast<uint8_t>(heights.front());
        r.bottom_margin = saturate_cast<uint8_t>(heights.back());
        r.left_margin = saturate_cast<uint8_t>(widths.front());
        r.right_margin = saturate_cast<uint8_t>(widths.back());

        auto total_width = size_excluding_margins(widths);
        r.emplace_back(total_width, size_excluding_margins(heights));

        auto i = 0_uz;
        while (inplace_next_height(heights, i)) {
            widths = get_widths(heights);
            if (compare_store(total_width, size_excluding_margins(widths))) {
                r.emplace_back(total_width, size_excluding_margins(heights));
            }
        }

        return r;
    }

private:
    cell_vector _cells = {};
    size_t _num_rows = 0;
    size_t _num_columns = 0;

    /** Sort the cells ordered by row then column.
     *
     * The ordering is the same as they keyboard focus chain order.
     */
    void sort_cells() noexcept
    {
        std::sort(_cells.begin(), _cells.end(), [](cell_type const& lhs, cell_type const& rhs) {
            if (lhs.first_row != rhs.first_row) {
                return lhs.first_row < rhs.first_row;
            } else {
                return lhs.first_column < rhs.first_column;
            }
        });
    }

    /** Updates needed after a cell was added or removed.
     */
    void update_after_insert_or_delete() noexcept
    {
        sort_cells();

        _num_rows = 0;
        _num_columns = 0;
        for (hilet& cell : _cells) {
            inplace_max(_num_rows, cell.last_row);
            inplace_max(_num_columns, cell.last_column);
        }
    }

    [[nodiscard]] static uint16_t size_of_span(std::vector<uint16_t> const& sizes, size_t first, size_t last) noexcept
    {
        hilet first_index = first * 2 + 1;
        hilet last_index = last * 2;

        return saturate_cast<uint16_t>(std::accumulate(sizes.begin() + first_index, sizes.begin() + last_index, size_t{0}));
    }

    [[nodiscard]] static uint16_t size_excluding_margins(std::vector<uint16_t> const& sizes) noexcept
    {
        hilet last = (sizes.size() - 1) / 2;
        return size_of_span(0, last);
    }

    static void
    inplace_expand_size_of_span(std::vector<uint16_t>& sizes, uint16_t needed_size, size_t first, size_t last) noexcept
    {
        // The total size of the span, including inner margins; excluding outer margins.
        hilet current_size = size_of_span(sizes, first, last);

        if (current_size >= needed_size) {
            // The span fits in the current size.
            return;
        }

        // The number of rows or columns of a span.
        hilet span = last - first;

        hilet extra_size = needed_size - current_size;
        hilet extra_size_per_item = extra_size / span;
        hilet extra_size_first_item = extra_size - extra_size_per_item * span;

        sizes[first * 2 + 1] += extra_size_first_item;
        for (auto i = first; i != last; ++i) {
            hilet index = i * 2 + 1;
            sizes[index] += extra_size_per_item;
        }
    }

    [[nodiscard]] std::vector<uint16_t> get_widths(std::vector<uint16_t> const& heights) const noexcept
    {
        // Every column with margins around every column.
        auto r = std::vector<uint16_t>(nr_columns() * 2 + 1);

        for (hilet& cell : _cells) {
            hilet left_margin_index = cell.first_column * 2;
            hilet right_margin_index = cell.last_column * 2;

            inplace_max(r[left_margin_index], cell.constraints.margin_left);
            inplace_max(r[right_margin_index], cell.constraints.marging_right);

            // Cells with rowspan=1 have a fixed width for each column.
            if (cell.colspan() == 1) {
                hilet span_height = height_of_span(heights, cell);
                hilet needed_width = cell.constraints.width_for_height(span_height);
                hilet column_index = cell.first_column * 2 + 1;

                inplace_max(r[column_index], needed_width);
            }
        }

        for (hilet& cell : _cells) {
            if (cell.colspan() > 1) {
                hilet span_height = size_of_span(heights, cell.first_row, cell.last_row);
                hilet needed_width = cell.constraints.width_for_height(span_height);

                inplace_expand_size_of_span(r, needed_width, cell.first_column, cell.last_column);
            }
        }

        return r;
    }

    /** Find the next height change.
     *
     * 1. Start of with the minimum heights (so maximum widths).
     * 2. For each column find the cell that caused the current width.
     *   1. Get the next smaller width of the cell.
     *   2. Find the cell that causes the smallest (but not zero) height change.
     *     1. Recalculate all the widths for just this cell change and return true.
     * 3. When all columns are minimum width return false.
     *
     * @param [in,out] widths The widths of each row.
     * @param [in,out] heights The heights of each row.
     * @retval true If there was a height change
     * @retval false There was no height change, stop iterating.
     */
    [[nodiscard]] bool inplace_next_smaller_width(std::vector<uint16_t>& widths, std::vector<uint16_t>& heights) noexcept
    {
        auto minimum_height_change = std::numeric_limits<uint16_t>::max();
        auto minimum_height_column_nr = 0_uz;

        for (auto column_nr = 0_uz; column_nr != num_columns(); ++column_nr) {
            auto column_width = widths[column_nr * 2 + 1];
            auto height_change = uint16_t{0};

            // Find all the cells that match the current column's width and see if changing
            // the call will cause a height change.
            for (hilet& cell : _cells) {
                if (cell.colspan() == 1 and cell.start_column == column_nr and cell.has_width(column_width)) {
                    if (auto optional_size = cell.next_smaller_width(column_width)) {
                        hilet old_span_height = size_of_span(heights, cell.first_row, cell.last_row);
                        if (optional_size->height > old_span_height) {
                            height_change += optional_size->height - old_span_height
                        }
                    }
                }
            }

            if (inplace_min(minimum_height_change, height_change)) {
                minimum_height_column_nr = column_nr;
            }
        }

        return false;
    }

    struct sizes_type {
        std::vector<uint16_t> widths;
        std::vector<uint16_t> heights;

        constexpr sizes_type(sizes_type const&) noexcept = default;
        constexpr sizes_type(sizes_type&&) noexcept = default;
        constexpr sizes_type& operator=(sizes_type const&) noexcept = default;
        constexpr sizes_type& operator=(sizes_type&&) noexcept = default;

        constexpr sizes_type(size_t num_columns, size_t num_rows) noexcept :
            widths(num_columns * 2 + 1), heights(num_rows * 2 + 1)
        {
        }
    };

    [[nodiscard]] sizes_type get_minimums() const noexcept
    {
        // Every row with margins around every row.
        auto r = sizes_type{num_columns(), num_rows()};

        // The margins are all fixed, and should be done first.
        for (hilet& cell : _cells) {
            inplace_max(r.heights[cell.first_row * 2], cell.constraints.margin_top);
            inplace_max(r.heights[cell.last_row * 2], cell.constraints.marging_bottom);
            inplace_max(r.widths[cell.first_column * 2], cell.constraints.margin_left);
            inplace_max(r.widths[cell.last_column * 2], cell.constraints.marging_right);

            if (cell.colspan() == 1) {
                inplace_max(r.widths[cell.first_column * 2 + 1], cell.minimum_width());
            }
            if (cell.rowspan() == 1) {
                inplace_max(r.heights[cell.first_row * 2 + 1], cell.minimum_height());
            }
        }

        for (hilet& cell : _cells) {
            if (cell.colspan() > 1) {
                inplace_expand_size(r.widths, cell.minimum_width(), cell.first_column, cell.last_column);
            }
            if (cell.rowspan() > 1) {
                inplace_expand_size(r.heights, cell.minimum_height(), cell.first_row, cell.last_row);
            }
        }
    }

    return r;
}
}}
} // namespace hi::v1
