

#pragma once

#include "grid_cell.hpp"

namespace hi { inline namespace v1 {

constexpr ~grid_cell::grid_cell()
{
    if (_grid) {
        _grid->remove_cell(_id);
        _grid->reconstrain = true;
    }
}

constexpr grid_cell::grid_cell(grid_cell&& other) noexcept : _grid(other._grid), _id(other._id)
{
    other._grid = nullptr;
}

constexpr grid_cell& grid_cell::operator=(grid_cell&& other) noexcept
{
    if (_grid) {
        _grid->remove_cell(_id);
        _grid->reconstrain = true;
    }

    _grid = std::exchange(other._grid, nullptr);
    _id = other._id;
}

constexpr grid_cell::grid_cell(grid const& grid) noexcept : _grid(std::addressof(grid)), _id(grid.add_cell())
{
    _grid->reconstrain = true;
}

[[nodiscard]] constexpr bool grid_cell::empty() const noexcept
{
    return _grid->at(_id).in_use == 0;
}

constexpr void grid_cell::clear() noexcept
{
    _grid->reconstrain |= compare_store(_grid->at(_id).in_use, 0);
}

constexpr void grid_cell::set_location(uint8_t col_begin, uint8_t row_begin, uint8_t col_end, uint8_t row_end) noexcept
{
    hi_axiom(col_begin < std::numeric_limits<uint8_t>::max());
    hi_axiom(row_begin < std::numeric_limits<uint8_t>::max());
    hi_axiom(col_begin < col_end);
    hi_axiom(row_begin < row_end);

    _grid->reconstrain |= compare_store(_grid->at(_id).col_begin, col_begin);
    _grid->reconstrain |= compare_store(_grid->at(_id).row_begin, row_begin);
    _grid->reconstrain |= compare_store(_grid->at(_id).col_end, col_end);
    _grid->reconstrain |= compare_store(_grid->at(_id).row_end, row_end);
}

constexpr void grid_cell::set_parent(grid_cell const& parent) noexcept
{
    _grid->reconstrain |= compare_store(_grid->at(_id).parent, parent._id);
}

constexpr void grid_cell::unset_parent() noexcept
{
    _grid->reconstrain |= compare_store((*_grid)[_id].parent, -1);
}

constexpr void grid_cell::set_priority(int8_t width_priority, int8_t height_priority) noexcept
{
    _grid->reconstrain |= compare_store((*_grid)[_id].width_priority, width_priority);
    _grid->reconstrain |= compare_store((*_grid)[_id].height_priority, height_priority);
}

constexpr void grid_cell::set_margin(hi::margins margin) noexcept
{
    _grid->reconstrain |= compare_store((*_grid)[_id].margin_left, round_cast<int8_t>(margin.left()));
    _grid->reconstrain |= compare_store((*_grid)[_id].margin_bottom, round_cast<int8_t>(margin.bottom()));
    _grid->reconstrain |= compare_store((*_grid)[_id].margin_right, round_cast<int8_t>(margin.right()));
    _grid->reconstrain |= compare_store((*_grid)[_id].margin_top, round_cast<int8_t>(margin.top()));
}

constexpr void grid_cell::set_size(hi::extent size) noexcept
{
    hi_axiom((*_grid)[_id].parent == -1);
    hi_axiom((*_grid)[_id].col_begin == 0);
    hi_axiom((*_grid)[_id].row_begin == 0);
    hi_axiom((*_grid)[_id].col_end == 1);
    hi_axiom((*_grid)[_id].row_end == 1);

    _grid->constrain();
    _grid-

    _grid->relayout |= compare_store((*_grid)[_id].width, round_cast<int32_t>(size.width()));
    _grid->relayout |= compare_store((*_grid)[_id].height, round_cast<int32_t>(size.height()));
}

[[nodiscard]] constexpr aarectangle grid_cell::rectangle() const noexcept
{
    _grid->layout();
}

}}
