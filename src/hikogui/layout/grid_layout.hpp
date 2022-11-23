// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "box_constraints.hpp"
#include "box_shape.hpp"
#include "spreadsheet_address.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../cast.hpp"
#include <cstdint>
#include <numeric>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

namespace hi { inline namespace v1 {

namespace detail {

enum class grid_layout_axis { x, x_rtol, y, y_btot };

enum class grid_layout_alignment { none, before, after, middle };

[[nodiscard]] constexpr vertical_alignment to_vertical_alignment(grid_layout_alignment const& rhs) noexcept
{
    switch (rhs) {
    case grid_layout_alignment::none:
        return vertical_alignment::none;
    case grid_layout_alignment::before:
        return vertical_alignment::top;
    case grid_layout_alignment::after:
        return vertical_alignment::bottom;
    case grid_layout_alignment::middle:
        return vertical_alignment::middle;
    }
    hi_no_default();
}

[[nodiscard]] constexpr horizontal_alignment to_horizontal_alignment(grid_layout_alignment const& rhs) noexcept
{
    switch (rhs) {
    case grid_layout_alignment::none:
        return horizontal_alignment::none;
    case grid_layout_alignment::before:
        return horizontal_alignment::left;
    case grid_layout_alignment::after:
        return horizontal_alignment::right;
    case grid_layout_alignment::middle:
        return horizontal_alignment::center;
    }
    hi_no_default();
}

[[nodiscard]] constexpr grid_layout_alignment to_grid_layout_alignment(horizontal_alignment rhs) noexcept
{
    switch (rhs) {
    case horizontal_alignment::none:
        return grid_layout_alignment::none;
    case horizontal_alignment::left:
        return grid_layout_alignment::before;
    case horizontal_alignment::right:
        return grid_layout_alignment::after;
    case horizontal_alignment::center:
        return grid_layout_alignment::middle;
    case horizontal_alignment::justified:
        return grid_layout_alignment::none;
    case horizontal_alignment::flush:
        // This should already been resolved.
        hi_no_default();
    }
    hi_no_default();
}

[[nodiscard]] constexpr grid_layout_alignment to_grid_layout_alignment(vertical_alignment rhs) noexcept
{
    switch (rhs) {
    case vertical_alignment::none:
        return grid_layout_alignment::none;
    case vertical_alignment::top:
        return grid_layout_alignment::before;
    case vertical_alignment::bottom:
        return grid_layout_alignment::after;
    case vertical_alignment::middle:
        return grid_layout_alignment::middle;
    }
    hi_no_default();
}

template<typename T>
struct grid_layout_cell {
    using value_type = T;

    size_t first_column = 0;
    size_t last_column = 0;
    size_t first_row = 0;
    size_t last_row = 0;
    value_type value = {};
    box_shape shape = {};

    constexpr grid_layout_cell() noexcept = default;
    constexpr grid_layout_cell(grid_layout_cell const&) noexcept = default;
    constexpr grid_layout_cell(grid_layout_cell&&) noexcept = default;
    constexpr grid_layout_cell& operator=(grid_layout_cell const&) noexcept = default;
    constexpr grid_layout_cell& operator=(grid_layout_cell&&) noexcept = default;

    constexpr grid_layout_cell(
        size_t first_column,
        size_t last_column,
        size_t first_row,
        size_t last_row,
        std::convertible_to<value_type> auto&& value) noexcept :
        first_column(first_column), last_column(last_column), first_row(first_row), last_row(last_row), value(hi_forward(value))
    {
    }

    constexpr void set_constraints(box_constraints const& constraints) noexcept
    {
        _constraints = constraints;
    }

    [[nodiscard]] constexpr size_t first(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
        case grid_layout_axis::x_rtol:
            return first_column;
        case grid_layout_axis::y:
        case grid_layout_axis::y_btot:
            return first_row;
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr size_t last(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
        case grid_layout_axis::x_rtol:
            return last_column;
        case grid_layout_axis::y:
        case grid_layout_axis::y_btot:
            return last_row;
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr size_t span(grid_layout_axis axis) const noexcept
    {
        return first(axis) - last(axis);
    }

    [[nodiscard]] constexpr grid_layout_alignment alignment(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
        case grid_layout_axis::x_rtol:
            return to_grid_layout_alignment(_constraints.alignment.horizontal());
        case grid_layout_axis::y:
        case grid_layout_axis::y_btot:
            return to_grid_layout_alignment(_constraints.alignment.vertical());
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr float minimum(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
        case grid_layout_axis::x_rtol:
            return std::ceil(_constraints.minimum.width());
        case grid_layout_axis::y:
        case grid_layout_axis::y_btot:
            return std::ceil(_constraints.minimum.height());
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr float preferred(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
        case grid_layout_axis::x_rtol:
            return std::ceil(_constraints.preferred.width());
        case grid_layout_axis::y:
        case grid_layout_axis::y_btot:
            return std::ceil(_constraints.preferred.height());
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr float maximum(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
        case grid_layout_axis::x_rtol:
            return std::floor(_constraints.maximum.width());
        case grid_layout_axis::y:
        case grid_layout_axis::y_btot:
            return std::floor(_constraints.maximum.height());
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr float margin_before(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
            return std::ceil(_constraints.margins.left());
        case grid_layout_axis::x_rtol:
            return std::ceil(_constraints.margins.right());
        case grid_layout_axis::y:
            return std::ceil(_constraints.margins.top());
        case grid_layout_axis::y_btot:
            return std::ceil(_constraints.margins.bottom());
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr float margin_after(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
            return std::ceil(_constraints.margins.right());
        case grid_layout_axis::x_rtol:
            return std::ceil(_constraints.margins.left());
        case grid_layout_axis::y:
            return std::ceil(_constraints.margins.bottom());
        case grid_layout_axis::y_btot:
            return std::ceil(_constraints.margins.top());
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr float padding_before(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
            return std::ceil(_constraints.padding.left());
        case grid_layout_axis::x_rtol:
            return std::ceil(_constraints.padding.right());
        case grid_layout_axis::y:
            return std::ceil(_constraints.padding.top());
        case grid_layout_axis::y_btot:
            return std::ceil(_constraints.padding.bottom());
        }
        hi_no_default();
    }

    [[nodiscard]] constexpr float padding_after(grid_layout_axis axis) const noexcept
    {
        switch (axis) {
        case grid_layout_axis::x:
            return std::ceil(_constraints.padding.right());
        case grid_layout_axis::x_rtol:
            return std::ceil(_constraints.padding.left());
        case grid_layout_axis::y:
            return std::ceil(_constraints.padding.bottom());
        case grid_layout_axis::y_btot:
            return std::ceil(_constraints.padding.top());
        }
        hi_no_default();
    }

private:
    box_constraints _constraints;
};

template<typename T>
class grid_layout_axis_constraints {
public:
    using value_type = T;
    using cell_type = grid_layout_cell<value_type>;
    using cell_vector = std::vector<cell_type>;

    struct constraint_type {
        float minimum = 0.0f;
        float preferred = 0.0f;
        float maximum = std::numeric_limits<float>::max();
        float margin_before = 0.0f;
        float padding_before = 0.0f;
        float padding_after = 0.0f;
        grid_layout_alignment alignment = grid_layout_alignment::none;
    };

    std::vector<constraint_type> constraints = {};
    size_t num = 0;
    float minimum = 0.0f;
    float preferred = 0.0f;
    float maximum = 0.0f;
    float margin_before = 0.0f;
    float margin_after = 0.0f;
    float padding_before = 0.0f;
    float padding_after = 0.0f;

    constexpr ~grid_layout_axis_constraints() = default;
    constexpr grid_layout_axis_constraints() noexcept = default;
    constexpr grid_layout_axis_constraints(grid_layout_axis_constraints const&) noexcept = default;
    constexpr grid_layout_axis_constraints(grid_layout_axis_constraints&&) noexcept = default;
    constexpr grid_layout_axis_constraints& operator=(grid_layout_axis_constraints const&) noexcept = default;
    constexpr grid_layout_axis_constraints& operator=(grid_layout_axis_constraints&&) noexcept = default;
    [[nodiscard]] constexpr friend bool
    operator==(grid_layout_axis_constraints const&, grid_layout_axis_constraints const&) noexcept = default;

    /** Construct constraints for this axis.
     *
     * @param cell The cells
     * @param num The number of cells in the direction of the current axis
     * @mirror true If the axis needs to be mirrored.
     */
    constexpr grid_layout_axis_constraints(cell_vector const& cells, size_t num, grid_layout_axis axis) noexcept :
        constraints(num + 1), num(num)
    {
        for (hilet& cell : cells) {
            simple_init_for_cell(cell, axis);
        }
        fixup();

        for (hilet& cell : cells) {
            expand_for_spans(cell, axis);
        }
        fixup();
        init_stats();
    }

    [[nodiscard]] constexpr constraint_type& operator[](size_t index) noexcept
    {
        return constraints[index];
    }

    [[nodiscard]] constexpr constraint_type const& operator[](size_t index) const noexcept
    {
        return constraints[index];
    }

    [[nodiscard]] constexpr constraint_type const& front() const noexcept
    {
        return constraints.front();
    }

    [[nodiscard]] constexpr constraint_type const& back() const noexcept
    {
        return constraints[constraints.size() - 2];
    }

    [[nodiscard]] constexpr std::tuple<float, float, float> const span_size(size_t first, size_t last) const noexcept
    {
        float r_minimum = 0.0f;
        float r_preferred = 0.0f;
        float r_maximum = 0.0f;
        float r_margin = 0.0f;
        if (first != last) {
            r_minimum = constraints[first].minimum;
            r_preferred = constraints[first].preferred;
            r_maximum = constraints[first].maximum;
            for (auto i = first + 1; i != last; ++i) {
                r_margin += constraints[i].margin_before;
                r_minimum += constraints[i].minimum;
                r_preferred += constraints[i].preferred;
                r_maximum += constraints[i].maximum;
            }
        }
        return {r_minimum + r_margin, r_preferred + r_margin, r_maximum + r_margin};
    }

    [[nodiscard]] constexpr std::tuple<float, float, float> const
    span_size(cell_type const& cell, grid_layout_axis axis) const noexcept
    {
        return span_size(cell.first(axis), cell.last(axis));
    }

    constexpr void simple_init_for_cell(cell_type const& cell, grid_layout_axis axis)
    {
        inplace_max(constraints[cell.first(axis)].margin_before, cell.margin_before(axis));
        inplace_max(constraints[cell.last(axis)].margin_before, cell.margin_after(axis));
        inplace_max(constraints[cell.first(axis)].padding_before, cell.padding_before(axis));
        inplace_max(constraints[cell.last(axis) - 1].padding_after, cell.padding_after(axis));

        if (cell.span(axis) == 1) {
            inplace_max(constraints[cell.first(axis)].alignment, cell.alignment(axis));
            inplace_max(constraints[cell.first(axis)].minimum, cell.minimum(axis));
            inplace_max(constraints[cell.first(axis)].preferred, cell.preferred(axis));
            inplace_min(constraints[cell.first(axis)].maximum, cell.maximum(axis));
        }
    }

    constexpr void expand_for_spans(cell_type const& cell, grid_layout_axis axis)
    {
        if (cell.span(axis) > 1) {
            hilet[span_minimum, span_preferred, span_maximum] = span_size(cell, axis);
            if (hilet extra = cell.minimum(axis) - span_minimum; extra > 0.0f) {
                hilet extra_per_cell = std::ceil(extra / cell.span(axis));
                for (auto i = cell.first(axis); i != cell.last(axis); ++i) {
                    constraints[i].minimum += extra_per_cell;
                }
            }

            if (hilet extra = cell.preferred(axis) - span_preferred; extra > 0.0f) {
                hilet extra_per_cell = std::ceil(extra / cell.span(axis));
                for (auto i = cell.first(axis); i != cell.last(axis); ++i) {
                    constraints[i].preferred += extra_per_cell;
                }
            }

            if (hilet extra = cell.maximum(axis) - span_preferred; extra < 0.0f) {
                hilet extra_per_cell = std::ceil(extra / cell.span(axis));
                for (auto i = cell.first(axis); i != cell.last(axis); ++i) {
                    // The maximum could become too low here, fixup() will fix this.
                    constraints[i].maximum += extra_per_cell;
                }
            }
        }
    }

    constexpr void fixup() noexcept
    {
        for (auto& row : constraints) {
            inplace_max(row.preferred, row.minimum);
            inplace_max(row.maximum, row.preferred);
            if (row.padding_before + row.padding_after > row.minimum) {
                hilet padding_diff = row.padding_after - row.padding_before;
                hilet middle = std::clamp(std::floor(row.minimum * 0.5f + padding_diff), 0.0f, row.minimum);
                row.padding_after = middle;
                row.padding_before = row.minimum - middle;
            }
        }
    }

    constexpr void init_stats() noexcept
    {
        std::tie(minimum, preferred, maximum) = span_size(0, num);
        margin_before = constraints.front().margin_before;
        margin_after = constraints.back().margin_before;
        padding_before = constraints.front().padding_before;
        padding_after = constraints[constraints.size() - 2].padding_after;
    }
};

} // namespace detail

template<typename T>
class grid_layout {
public:
    using value_type = T;

    using cell_type = detail::grid_layout_cell<value_type>;
    using cell_vector = std::vector<cell_type>;
    using iterator = cell_vector::iterator;
    using const_iterator = cell_vector::const_iterator;
    using reference = cell_vector::reference;

    ~grid_layout() = default;
    constexpr grid_layout() noexcept = default;
    constexpr grid_layout(grid_layout const&) noexcept = default;
    constexpr grid_layout(grid_layout&&) noexcept = default;
    constexpr grid_layout& operator=(grid_layout const&) noexcept = default;
    constexpr grid_layout& operator=(grid_layout&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(grid_layout const&, grid_layout const&) noexcept = default;

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
    [[nodiscard]] constexpr bool cell_in_use(size_t first_column, size_t last_column, size_t first_row, size_t last_row) noexcept
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
     * @param first_column The first column of the cell-span.
     * @param last_column One beyond the last column of the cell-span.
     * @param first_row The first row of the cell-span.
     * @param last_row One beyond the last row of the cell-span.
     * @param value The value to be copied or moved into the cell.
     * @return A reference to the created cell.
     */
    template<forward_of<value_type> Value>
    constexpr reference
    add_cell(size_t first_column, size_t last_column, size_t first_row, size_t last_row, Value&& value) noexcept
    {
        // At least one cell must be in the range.
        hi_axiom(first_column < last_column);
        hi_axiom(first_row < last_row);
        hi_axiom(not cell_in_use(first_column, last_column, first_row, last_row));
        auto& r = _cells.emplace_back(first_column, last_column, first_row, last_row, std::forward<Value>(value));
        update_after_insert_or_delete();
        return r;
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param column The column of the cell.
     * @param row The row of the cell.
     * @param value The value to be copied or moved into the cell.
     * @return A reference to the created cell.
     */
    template<forward_of<value_type> Value>
    constexpr reference add_cell(size_t column, size_t row, Value&& value) noexcept
    {
        return add_cell(column, column + 1, row, row + 1, std::forward<Value>(value));
    }

    constexpr void clear() noexcept
    {
        _cells.clear();
        update_after_insert_or_delete();
    }

    [[nodiscard]] constexpr box_constraints get_constraints(bool left_to_right) const noexcept
    {
        _row_constraints = detail::grid_layout_axis_constraints{_cells, num_rows(), detail::grid_layout_axis::y};
        _column_constraints = detail::grid_layout_axis_constraints{
            _cells, num_columns(), left_to_right ? detail::grid_layout_axis::x : detail::grid_layout_axis::x_rtol};

        hilet minimum_size = extent2{_column_constraints.minimum, _row_constraints.minimum};
        hilet preferred_size = extent2{_column_constraints.preferred, _row_constraints.preferred};
        hilet maximum_size = extent2{_column_constraints.maximum, _row_constraints.maximum};

        hilet margin_left = left_to_right ? _column_constraints.margin_before : _column_constraints.margin_after;
        hilet margin_bottom = _row_constraints.margin_after;
        hilet margin_right = left_to_right ? _column_constraints.margin_after : _column_constraints.margin_before;
        hilet margin_top = _row_constraints.margin_before;
        hilet margins = hi::margins{margin_left, margin_bottom, margin_right, margin_top};

        hilet padding_left = left_to_right ? _column_constraints.padding_before : _column_constraints.padding_after;
        hilet padding_bottom = _row_constraints.padding_after;
        hilet padding_right = left_to_right ? _column_constraints.padding_after : _column_constraints.padding_before;
        hilet padding_top = _row_constraints.padding_before;
        hilet padding = hi::margins{padding_left, padding_bottom, padding_right, padding_top};

        hilet alignment = [&] {
            if (num_rows() == 1 and num_columns() == 1) {
                return hi::alignment{
                    to_horizontal_alignment(_column_constraints.front().alignment),
                    to_vertical_alignment(_row_constraints.front().alignment)};
            } else if (num_rows() == 1) {
                return hi::alignment{to_vertical_alignment(_row_constraints.front().alignment)};
            } else if (num_columns() == 1) {
                return hi::alignment{to_horizontal_alignment(_column_constraints.front().alignment)};
            } else {
                return hi::alignment{};
            }
        }();

        return box_constraints{minimum_size, preferred_size, maximum_size, alignment, margins, padding};
    }

    constexpr void set_layout(extent2 size) noexcept
    {
        hi_not_implemented();
    }

private:
    cell_vector _cells = {};
    size_t _num_rows = 0;
    size_t _num_columns = 0;
    mutable detail::grid_layout_axis_constraints<value_type> _row_constraints = {};
    mutable detail::grid_layout_axis_constraints<value_type> _column_constraints = {};

    /** Sort the cells ordered by row then column.
     *
     * The ordering is the same as they keyboard focus chain order.
     */
    constexpr void sort_cells() noexcept
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
    constexpr void update_after_insert_or_delete() noexcept
    {
        sort_cells();

        _num_rows = 0;
        _num_columns = 0;
        for (hilet& cell : _cells) {
            inplace_max(_num_rows, cell.last_row);
            inplace_max(_num_columns, cell.last_column);
        }
    }
};

}} // namespace hi::v1
