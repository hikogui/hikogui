// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_layout.hpp"
#include "../required.hpp"
#include "../math.hpp"

namespace tt::inline v1 {

/** Grow the cells in a span.
 *
 * @param first The iterator pointing to the first cell to grow.
 * @param last The iterator pointing one beyond the last cell to grow.
 * @param growth The amount by which the total span of cells must grow
 * @param predicate A function with signature `float(grid_layout::cell_type const &)` which returns
 *        the maximum growth that the cell will allow.
 * @return The amount of growth left over.
 */
[[nodiscard]] static float grow(auto first, auto last, float growth, auto const &predicate) noexcept
{
    tt_axiom(growth >= 0.0f);

    auto num_cells_to_grow = std::count_if(first, last, [&predicate](ttlet &cell) {
        return predicate(cell) > 0.0f;
    });
    while (growth > 0.0f and num_cells_to_grow != 0) {
        // Grow each cell
        ttlet growth_per_cell = std::ceil(growth / num_cells_to_grow);

        // In the loop we count the number of cells that can grow again in the next iteration.
        num_cells_to_grow = 0_uz;
        for (auto it = first; it != last; ++it) {
            ttlet growth_this_cell = std::min(predicate(*it), std::min(growth_per_cell, growth));

            if (growth_this_cell > 0.0f) {
                it->size += growth_this_cell;
                growth -= growth_this_cell;

                // Check if this cell can grow in the next iteration.
                if (predicate(*it) > 0.0f) {
                    ++num_cells_to_grow;
                }
            }
        }
    }
    return growth;
}

static void grow(auto first, auto last, float growth) noexcept
{
    // First grow the cells to their preferred size.
    growth = grow(first, last, growth, [](ttlet &cell) {
        return cell.preferred - cell.size;
    });
    if (growth == 0.0f) {
        return;
    }

    // Next grow to cell to their maximum size.
    growth = grow(first, last, growth, [](ttlet &cell) {
        return cell.maximum - cell.size;
    });
    if (growth == 0.0f) {
        return;
    }

    // At this point we will need to violate the maximum size constraint of a cell.
    // First do this for cells that already have a maximum that is different from their preferred.
    // All cells where `preferred < maximum` will absorb all of the growth.
    growth = grow(first, last, growth, [](ttlet &cell) {
        return cell.maximum - cell.preferred;
    });
    if (growth == 0.0f) {
        return;
    }

    // Fallback by growing all of the widgets.
    growth = grow(first, last, growth, [growth](ttlet &cell) {
        return growth;
    });

    tt_axiom(growth == 0.0f);
}

void grid_layout::constrain_cells_by_singles() noexcept
{
    for (ttlet &constraint : _constraints) {
        inplace_max(_cells[constraint.first].margin, constraint.margin_before);
        inplace_max(_cells[constraint.last].margin, constraint.margin_after);

        if (constraint.is_single_cell()) {
            _cells[constraint.first].set_constraint(constraint);
        }
    }

    // Due to the calculations above, make sure minimum <= preferred <= maximum
    for (auto &cell : _cells) {
        cell.fix_constraint();
        tt_axiom(cell.holds_invariant());
    }
}

[[nodiscard]] void
grid_layout::constrain_cells_by_spans(std::function<float(grid_layout::constraint_type const &)> const &predicate) noexcept
{
    for (ttlet &constraint : _constraints) {
        if (constraint.is_span()) {
            ttlet first = _cells.begin() + constraint.first;
            ttlet last = _cells.begin() + constraint.last;
            ttlet size = get_size(first, last);
            if (ttlet extra_size = predicate(constraint) - size; extra_size > 0.0f) {
                grow(first, last, extra_size);
            }
        }
    }
}

void grid_layout::commit_constraints() noexcept
{
    // Add one more cell to handle the end margin.
    tt_axiom(_cells.empty());
    _cells.resize(num_cells() + 1);

    constrain_cells_by_singles();

    // Start off with the cells set to their minimum size based on single cell size.
    for (auto &cell : _cells) {
        cell.size = cell.minimum;
    }

    // Calculate the minimum cell sizes based on cell spans.
    constrain_cells_by_spans([](ttlet &constraint) {
        return constraint.minimum;
    });
    _minimum = get_size(0, num_cells());

    // Now that we know the actual minimum size of the cells, prepare for the preferred size calculation.
    for (auto &cell : _cells) {
        cell.minimum = cell.size;
        inplace_max(cell.maximum, cell.minimum);
        inplace_clamp(cell.preferred, cell.minimum, cell.maximum);
        cell.size = cell.preferred;
    }

    // Calculate the preferred cell sizes based on cell spans.
    constrain_cells_by_spans([](ttlet &constraint) {
        return constraint.preferred;
    });
    _preferred = get_size(0, num_cells());

    // Now that we know the actual preferred size of the cells, prepare for the maximum size calculation.
    for (auto &cell : _cells) {
        cell.preferred = cell.size;
        cell.maximum = std::max(cell.maximum, cell.preferred);
        cell.size = cell.maximum;
    }

    // Calculate the maximum cell sizes based on cell spans.
    constrain_cells_by_spans([](ttlet &constraint) {
        return constraint.maximum;
    });
    _maximum = get_size(0, num_cells());

    // Now we know the actual maximum size of cells.
    for (auto &cell : _cells) {
        cell.maximum = cell.size;
        tt_axiom(cell.holds_invariant());
    }
}

[[nodiscard]] bool grid_layout::holds_invariant() const noexcept
{
    for (ttlet &constraint: _constraints) {
        if (get_size(constraint.first, constraint.last) < constraint.minimum) {
            return false;
        }
    }
    return true;
}

void grid_layout::layout(float size) noexcept
{
    tt_axiom(size >= minimum());

    // Reset layout to the minimum size.
    for (auto &cell: _cells) {
        cell.size = cell.minimum;
    }

    // Include the last margin.
    if (ttlet needed_growth = size - minimum(); needed_growth > 0.0f) {
        grow(_cells.begin(), _cells.begin() + num_cells(), needed_growth);
    }

    tt_axiom(holds_invariant());
}

} // namespace tt::inline v1
