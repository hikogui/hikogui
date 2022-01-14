// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include "../math.hpp"
#include <tuple>
#include <vector>
#include <cstddef>
#include <functional>

namespace tt::inline v1 {

/** Grid layout is used to layout widgets along an axis.
 *
 * A `grid_widget` will use two `grid_layout`, one for column and one for row layout.
 * The `row_widget` and `column_widget` only need a single `grid_layout`.
 */
class grid_layout {
public:
    grid_layout(grid_layout const &) = delete;
    grid_layout(grid_layout &&) = delete;
    grid_layout &operator=(grid_layout const &) = delete;
    grid_layout &operator=(grid_layout &&) = delete;

    grid_layout() noexcept {}

    /** Clear the list of widgets in the layout.
     */
    void clear() noexcept
    {
        _num_cells = 0;
        _constraints.clear();
        _cells.clear();
    }

    /** Add a constraint for a widget.
     *
     * @param first The index of the first cell that the widget occupies.
     * @param last The index one past the last cell that the widget occupies.
     * @param minimum The absolute minimum size that a widget must be laid out as.
     * @param preferred The preferred size a widgets wants to be laid out as.
     * @param maximum The maximum size that a widget should be laid out as.
     * @param margin_before The space between this widget and other widgets.
     * @param margin_after The space between this widget and other widgets.
     */
    void add_constraint(std::size_t first, std::size_t last, float minimum, float preferred, float maximum, float margin_before, float margin_after) noexcept
    {
        _num_cells = std::max(_num_cells, last);
        _constraints.emplace_back(first, last, minimum, preferred, maximum, margin_before, margin_after);
    }

    /** Add a constraint for a widget.
     *
     * @param index The index of the cell that the widget occupies.
     * @param minimum The absolute minimum size that a widget must be laid out as.
     * @param preferred The preferred size a widgets wants to be laid out as.
     * @param maximum The maximum size that a widget should be laid out as.
     * @param margin_before The space between this widget and other widgets.
     * @param margin_after The space between this widget and other widgets.
     */
    void add_constraint(std::size_t index, float minimum, float preferred, float maximum, float margin_before, float margin_after) noexcept
    {
        return add_constraint(index, index + 1, minimum, preferred, maximum, margin_before, margin_after);
    }

    /** Commit all the constraints.
     *
     * This function will start calculating the constraints of the grid_layout.
     *
     * @pre All constraints have been set using `add_constraints()`.
     * @note It is undefined behavior when a widget is added more than once.
     * @note It is undefined behavior when a cell in a sequence is unused.
     */
    void commit_constraints() noexcept;

    /** The number of cells of the grid_layout.
     *
     * @pre `commit_constraints()` must be called.
     */
    [[nodiscard]] std::size_t num_cells() const noexcept
    {
        return _num_cells;
    }

    /** The minimum size of the total grid_layout.
     *
     * @pre `commit_constraints()` must be called.
     */
    [[nodiscard]] float minimum() const noexcept
    {
        return _minimum;
    }

    /** The minimum size of the total grid_layout.
     *
     * @pre `commit_constraints()` must be called.
     */
    [[nodiscard]] float preferred() const noexcept
    {
        return _preferred;
    }

    /** The minimum size of the total grid_layout.
     *
     * @pre `commit_constraints()` must be called.
     */
    [[nodiscard]] float maximum() const noexcept
    {
        return _maximum;
    }

    [[nodiscard]] float margin_before() const noexcept
    {
        return _cells.front().margin;
    }

    [[nodiscard]] float margin_after() const noexcept
    {
        return _cells.back().margin;
    }

    /** Layout the cells based on the total size.
     *
     * @pre `commit_constraints()` must be called.
     */
    void layout(float size) noexcept;

    /** Get position of cell.
     *
     * @param index The index of the cell.
     * @return The lower position of the cell, excluding the cell's margin.
     */
    [[nodiscard]] float get_position(std::size_t index) const noexcept
    {
        tt_axiom(index < num_cells());
        return get_position(_cells.begin(), _cells.begin() + index);
    }

    /** Get size of the cells.
     *
     * @param first The index of the first cell.
     * @param last The index one past the last cell.
     * @return The size of the cell-span excluding external margins.
     */
    [[nodiscard]] float get_size(std::size_t first, std::size_t last) const noexcept
    {
        tt_axiom(first <= last);
        tt_axiom(last <= _cells.size());
        return get_size(_cells.begin() + first, _cells.begin() + last);
    }

    /** Get size of the cell.
     *
     * @param index The index of the cell.
     * @return The size of the cell-span excluding external margins.
     */
    [[nodiscard]] float get_size(std::size_t index) const noexcept
    {
        return get_size(index, index + 1);
    }

    /** Get the position and size of a cell-span.
     *
     * @param first The index of the first cell.
     * @param last The index one past the last cell.
     * @return The position and size of the cell-span excluding external margins.
     */
    std::pair<float, float> get_position_and_size(std::size_t first, std::size_t last) const noexcept
    {
        return {get_position(first), get_size(first, last)};
    }

    /** Get the position and size of cell.
     *
     * @param index The index of the cell.
     * @return The position and size of the cell-span excluding external margins.
     */
    std::pair<float, float> get_position_and_size(std::size_t index) const noexcept
    {
        return get_position_and_size(index, index + 1);
    }

    /** Get the start and end position of the cells.
     *
     * @param first The index of the first cell.
     * @param last The index one past the last cell.
     * @return The first and last position of the cell-span excluding external margins.
     */
    std::pair<float, float> get_positions(std::size_t first, std::size_t last) const noexcept
    {
        ttlet position = get_position(first);
        ttlet size = get_size(first, last);
        return {position, position + size};
    }

    /** Get the start and end position of a cell.
     *
     * @pre `layout()` must be called.
     * @param index The index of the cell.
     * @return The first and last position of the cell excluding external margins.
     */
    std::pair<float, float> get_positions(std::size_t index) const noexcept
    {
        return get_positions(index, index + 1);
    }

private:
    struct constraint_type {
        std::size_t first;
        std::size_t last;
        float minimum;
        float preferred;
        float maximum;
        float margin_before;
        float margin_after;

        [[nodiscard]] bool is_single_cell() const noexcept
        {
            return first == last - 1;
        }

        [[nodiscard]] bool is_span() const noexcept
        {
            return not is_single_cell();
        }
    };

    struct cell_type {
        /** The size of this cell.
         */
        float size;

        /** The margin before the cell.
         */
        float margin;

        /** The absolute minimum size of this widget.
         */
        float minimum;

        /** The preferred size of this cell.
         */
        float preferred;

        /** The maximum size of this cell.
         */
        float maximum;

        cell_type() noexcept :
            size(0.0f), margin(0.0f), minimum(0.0f), preferred(0.0f), maximum(std::numeric_limits<float>::infinity())
        {
        }

        void fix_constraint() noexcept
        {
            inplace_max(maximum, minimum);
            inplace_clamp(preferred, minimum, maximum);
        }

        void set_constraint(constraint_type const &constraint) noexcept
        {
            inplace_max(minimum, constraint.minimum);
            inplace_max(preferred, constraint.preferred);
            inplace_min(maximum, constraint.maximum);
        }

        [[nodiscard]] bool holds_invariant() const noexcept
        {
            return minimum <= preferred and preferred <= maximum;
        }
    };

    using contraint_vector_type = std::vector<constraint_type>;
    using cell_vector_type = std::vector<cell_type>;
    using cell_iterator = cell_vector_type::iterator;
    using cell_const_iterator = cell_vector_type::const_iterator;

    std::size_t _num_cells = 0;
    float _minimum = 0.0f;
    float _preferred = 0.0f;
    float _maximum = 0.0f;
    contraint_vector_type _constraints;
    cell_vector_type _cells;

    /** Get position of cell.
     *
     * @param begin An iterator to the first cell.
     * @param first An iterator to the cell to calculate the position for.
     * @return The lower position of the cell, after the cell's margin, skipping the first margin.
     */
    [[nodiscard]] float get_position(cell_const_iterator begin, cell_const_iterator first) const noexcept
    {
        auto it = begin;
        auto position = 0.0f;
        while (it != first) {
            position += it->size;
            ++it;
            position += it->margin;
        }
        return position;
    }

    /** Get size of the cells.
     *
     * @param first The index of the first cell.
     * @param last The index one past the last cell.
     * @return The size of the cell-span excluding external margins.
     */
    [[nodiscard]] static float get_size(cell_const_iterator first, cell_const_iterator last) noexcept
    {
        if (first == last) {
            return 0.0f;
        }

        auto it = first;
        auto size = it->size;
        ++it;
        for (; it != last; ++it) {
            size += it->margin;
            size += it->size;
        }
        return size;
    }

    void constrain_cells_by_singles() noexcept;
    void constrain_cells_by_spans(std::function<float(constraint_type const &)> const &predicate) noexcept;
    [[nodiscard]] bool holds_invariant() const noexcept;
};

} // namespace tt::inline v1
