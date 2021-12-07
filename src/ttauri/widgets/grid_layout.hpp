// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

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
        _widget_constraints.clear();
        _cells.clear();
        _margins.clear();
    }

    /** Add a constraint for a widget.
     *
     * @param first The index of the first cell that the widget occupies.
     * @param last The index one past the last cell that the widget occupies.
     * @param minimum The absolute minimum size that a widget must be laid out as.
     * @param preferred The preferred size a widgets wants to be laid out as.
     * @param maximum The maximum size that a widget should be laid out as.
     * @param margin The space between this widget and other widgets.
     */
    void add_constraint(size_t first, size_t last, float minimum, float preferred, float maximum, float margin) noexcept
    {
        _num_cells = std::max(_num_cells, last);
        _constraints.emplace_back(first, last, minimum, preferred, maximum, margin);
    }

    /** Add a constraint for a widget.
     *
     * @param index The index of the cell that the widget occupies.
     * @param minimum The absolute minimum size that a widget must be laid out as.
     * @param preferred The preferred size a widgets wants to be laid out as.
     * @param maximum The maximum size that a widget should be laid out as.
     * @param margin The space between this widget and other widgets.
     */
    void add_constraint(size_t index, float minimum, float preferred, float maximum, float margin) noexcept
    {
        return add_constraint(index, index + 1, minimum, preferred, maximum, margin);
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
    [[nodiscard]] size_t num_cells() const noexcept
    {
        return _num_cells;
    }

    /** The minimum size of the total grid_layout.
     *
     * @pre `commit_constraints()` must be called.
     */
    [[nodiscard]] float minimum() const noexcept;

    /** The minimum size of the total grid_layout.
     *
     * @pre `commit_constraints()` must be called.
     */
    [[nodiscard]] float preferred() const noexcept;

    /** The minimum size of the total grid_layout.
     *
     * @pre `commit_constraints()` must be called.
     */
    [[nodiscard]] float maximum() const noexcept;

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
    [[nodiscard]] float get_position(size_t index) const noexcept
    {
        auto i = 0_uz;
        auto position = _cells[i].margin;
        for (; i != index; ++i) {
            position += cells[i].size;
            position += cells[i + 1].margin;
        }
        return position;
    }

    /** Get size of the cells.
     *
     * @param first The index of the first cell.
     * @param last The index one past the last cell.
     * @return The size of the cell-span excluding external margins.
     */
    [[nodiscard]] float get_size(size_t first, size_t last) const noexcept
    {
        tt_axiom(first < last);

        auto i = first;
        auto size = _cells[i].size;
        for (; i != last; ++i) {
            size += cells[i].margin size += cells[i].size;
        }
    }

    /** Get size of the cell.
     *
     * @param index The index of the cell.
     * @return The size of the cell-span excluding external margins.
     */
    [[nodiscard]] float get_size(size_t index) const noexcept
    {
        return get_size(index, index + 1);
    }

    /** Get the start and end position of the cells.
     *
     * @param first The index of the first cell.
     * @param last The index one past the last cell.
     * @return The first and last position of the cell-span excluding external margins.
     */
    std::pair<float, float> get_positions(size_t first, size_t last) const noexcept
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
    std::pair<float, float> get_positions(size_t index) const noexcept
    {
        return get_position(index, index + 1);
    }

private:
    struct constraints_type {
        size_t start;
        size_t end;
        float minimum;
        float preferred;
        float maximum;
        float margin;

        [[nodiscard]] bool is_single_cell() const noexcept
        {
            return start == end - 1;
        }

        [[nodiscard]] bool is_span() const noexcept
        {
            return not is_single_cell();
        }
    };

    struct cell_type {
        float size;

        /** The margin left of the cell.
         */
        float margin;
        float minimum;
        float preferred;
        float maximum;

        cell_type() noexcept : size(0.0f), margin(0.0f), minimum(0.0f), preferred(0.0f), maximum(0.0f) {}

        void fix_constraints() noexcept
        {
            inplace_max(maximum, minimum);
            inplace_clamp(preferred, minimum, maximum);
        }

        void set_constraints(constraint_type const &constraint) noexcept
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

    using contraints_vector_type = std::vector<constraints_type>
    using cell_vector_type = std::vector<cell_type>;
    using cell_iterator = cell_vector_type::iterator;

    size_t _num_cells;
    float _minimum;
    float _preferred;
    float _maximum;
    contraints_vector_type _constraints;
    cell_vector_type _cells;

    [[nodiscard]] static bool has_preferred_room(cell_iterator first, cell_iterator last) noexcept;
    [[nodiscard]] static bool has_maximum_room(cell_iterator first, cell_iterator last) noexcept;
    [[nodiscard]] static float add_upto_preferred_size(cell_iterator first, cell_iterator last, float extra) noexcept;
    [[nodiscard]] static float add_upto_maximum_size(cell_iterator first, cell_iterator last, float extra) noexcept;
    static void add_beyond_maximum_size(cell_iterator first, cell_iterator last, float extra) noexcept;
    static void add_size(cell_iterator first, cell_iterator last, float extra_size) noexcept;

    void cells_update_constraints_and_margins() noexcept;
    [[nodiscard]] float cells_calculate_minimum() noexcept;
    [[nodiscard]] float cells_calculate_preferred() noexcept;
    [[nodiscard]] float cells_calculate_maximum() noexcept;
};

} // namespace tt::inline v1
