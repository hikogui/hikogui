// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_layout_widget.hpp"
#include "../algorithm.hpp"
#include "../alignment.hpp"

namespace tt {

grid_layout_widget::grid_layout_widget(
    gui_window &window,
    std::shared_ptr<widget> parent,
    std::shared_ptr<widget_delegate> delegate) noexcept :
    widget(window, std::move(parent), std::move(delegate))
{
    if (auto p = _parent.lock()) {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        _semantic_layer = p->semantic_layer();
    }
    _margin = 0.0f;
}

[[nodiscard]] std::pair<size_t, size_t> grid_layout_widget::calculate_grid_size(std::vector<cell> const &cells) noexcept
{
    size_t nr_columns = 0;
    size_t nr_rows = 0;

    for (auto &&cell : cells) {
        nr_rows = std::max(nr_rows, cell.row_nr + 1);
        nr_columns = std::max(nr_columns, cell.column_nr + 1);
    }

    return {nr_columns, nr_rows};
}

[[nodiscard]] std::tuple<extent2, extent2, extent2>
grid_layout_widget::calculate_size(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    rows.clear();
    columns.clear();

    ttlet[nr_columns, nr_rows] = calculate_grid_size(cells);
    rows.reserve(nr_rows);
    columns.reserve(nr_columns);

    ttlet max_row_nr = nr_rows - 1;
    for (auto &&cell : cells) {
        rows.update(
            cell.row_nr,
            cell.widget->minimum_size().height(),
            cell.widget->preferred_size().height(),
            cell.widget->maximum_size().height(),
            cell.widget->margin());

        columns.update(
            cell.column_nr,
            cell.widget->minimum_size().width(),
            cell.widget->preferred_size().width(),
            cell.widget->maximum_size().width(),
            cell.widget->margin());
    }

    return {
        extent2{columns.minimum_size(), rows.minimum_size()},
        extent2{columns.preferred_size(), rows.preferred_size()},
        extent2{columns.maximum_size(), rows.maximum_size()}};
}

bool grid_layout_widget::address_in_use(size_t column_nr, size_t row_nr) const noexcept
{
    for (ttlet &cell : _cells) {
        if (cell.column_nr == column_nr && cell.row_nr == row_nr) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<widget> grid_layout_widget::add_widget(size_t column_nr, size_t row_nr, std::shared_ptr<widget> widget) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    auto tmp = widget::add_widget(std::move(widget));

    tt_assert(!address_in_use(column_nr, row_nr), "cell ({},{}) of grid_widget is already in use", column_nr, row_nr);

    _cells.emplace_back(column_nr, row_nr, tmp);
    return tmp;
}

bool grid_layout_widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (super::update_constraints(display_time_point, need_reconstrain)) {
        std::tie(_minimum_size, _preferred_size, _maximum_size) = calculate_size(_cells, _rows, _columns);
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

void grid_layout_widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
    if (need_layout) {
        _columns.set_size(width());
        _rows.set_size(height());

        for (auto &&cell : _cells) {
            auto &&child = cell.widget;
            ttlet child_rectangle = cell.rectangle(_columns, _rows, height());
            child->set_layout_parameters_from_parent(child_rectangle);
        }
    }

    super::update_layout(display_time_point, need_layout);
}

} // namespace tt
