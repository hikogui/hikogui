// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_layout.hpp"
#include "../required.hpp"
#include "../math.hpp"

namespace tt::inline v1 {

void grid_layout::cells_update_constraints_and_margins() noexcept
{
    for (ttlet &constraint : constraints) {
        inplace_max(_cells[constraint.first].margin, constraint.margin);
        inplace_max(_cells[constraint.last].margin, constraint.margin);

        if (constraints.is_single_cell()) {
            _cells[constraint.first].set_constraints(constraints);
        }
    }

    for (auto &cell : _cells) {
        cell.fix_constraints();
        tt_axiom(cell.holds_invariant());
    }
}

[[nodiscard]] bool grid_layout::has_preferred_room(grid_layout::cell_iterator first, grid_layout::cell_iterator last) noexcept
{
    for (auto it = first; it != last; ++it) {
        if (it->size < it->preferred) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool grid_layout::has_maximum_room(grid_layout::cell_iterator first, grid_layout::cell_iterator last) noexcept
{
    for (auto it = first; it != last; ++it) {
        if (it->size < it->maximum) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] float
grid_layout::add_upto_preferred_size(grid_layout::cell_iterator first, grid_layout::cell_iterator last, float extra) noexcept
{
    tt_axiom(extra >= 0.0f);

    ttlet extra_per_cell = std::ceil(extra / span);
    for (auto it = first; it != last; ++it) {
        ttlet room_this_cell = std::max(0.0f, it->preferred - it->size);
        ttlet extra_this_cell = std::clamp(extra_per_cell, 0.0f, std::min(extra, room_this_cell));
        it->size += extra_this_cell;
        extra -= extra_this_cell;
    }
    return extra;
}

[[nodiscard]] float
grid_layout::add_upto_maximum_size(grid_layout::cell_iterator first, grid_layout::cell_iterator last, float extra) noexcept
{
    tt_axiom(extra >= 0.0f);

    ttlet extra_per_cell = std::ceil(extra / span);
    for (auto it = first; it != last; ++it) {
        ttlet room_this_cell = std::max(0.0f, it->maximum - it->size);
        ttlet extra_this_cell = std::clamp(extra_per_cell, 0.0f, std::min(extra, room_this_cell));
        it->size += extra_this_cell;
        extra -= extra_this_cell;
    }
    return extra;
}

void grid_layout::add_beyond_maximum_size(grid_layout::cell_iterator first, grid_layout::cell_iterator last, float extra) noexcept
{
    tt_axiom(extra >= 0.0f);
    ttlet extra_per_cell = std::ceil(extra / span);

    for (auto it = first; it != last; ++it) {
        ttlet extra_this_cell = std::min(extra_per_cell, extra);
        it->size += extra_this_cell;
        extra -= extra_this_cell;
    }
    return extra;
}

void grid_layout::add_size(grid_layout::cell_iterator first, grid_layout::cell_iterator last, float extra) noexcept
{
    ttlet span = std::distance(first, last);

    while (extra > 0.0f and has_preferred_room(first, last)) {
        extra = add_upto_preferred_size(first, last, extra);
    }

    while (extra > 0.0f and has_maximum_room(first, last)) {
        extra = add_upto_maximum_size(first, last, extra);
    }

    add_beyond_maximum_size(first, last, extra);
}

[[nodiscard]] float grid_layout::cells_calculate_minimum() noexcept
{
    for (auto &cell : _cells) {
        cell.size = cell.minimum;
    }

    for (ttlet &constraint : _constraints) {
        if (constraint.is_span()) {
            ttlet size = get_size(constraint.first, constraint.last);
            if (ttlet extra_size = constraint.minimum - size; extra_size > 0.0f) {
                add_size(constraint.first, constraint.last, extra_size);
            }
        }
    }

    return get_size(0, num_cells());
}

[[nodiscard]] float grid_layout::cells_calculate_preferred() noexcept
{
    for (auto &cell : _cells) {
        cell.size = cell.preferred;
    }

    for (ttlet &constraint : _constraints) {
        if (constraint.is_span()) {
            ttlet size = get_size(constraint.first, constraint.last);
            if (ttlet extra_size = constraint.preferred - size; extra_size > 0.0f) {
                add_size(constraint.first, constraint.last, extra_size);
            }
        }
    }

    return get_size(0, num_cells());
}

[[nodiscard]] float grid_layout::cells_calculate_maximum() noexcept
{
    for (auto &cell : _cells) {
        cell.size = cell.maximum;
    }

    for (ttlet &constraint : _constraints) {
        if (constraint.is_span()) {
            ttlet size = get_size(constraint.first, constraint.last);
            if (ttlet extra_size = constraint.maximum - size; extra_size > 0.0f) {
                add_size(constraint.first, constraint.last, extra_size);
            }
        }
    }

    return get_size(0, num_cells());
}

void grid_layout::commit_constraints() noexcept
{
    // Add one more cell to handle the end margin.
    tt_axiom(_cells.empty());
    _cells.resize(num_cells() + 1);

    cells_update_constraints_and_margins();

    _minimum = cells_calculate_minimum();
    _preferred = cells_calculate_minimum();
    _maximum = cells_calculate_minimum();
}

void grid_layout::layout(float size) noexcept
{
    // Reset layout to the minimum size.
    _minimum = cells_calculate_minimum();
    tt_axiom(size >= _minimum);

    // Include the last margin.
    if (ttlet extra_size = size - _minimum; extra_size > 0.0f) {
        add_size(_cells.begin(), _cells.begin() + num_cells(), extra_size);
    }
}

} // namespace tt::inline v1
