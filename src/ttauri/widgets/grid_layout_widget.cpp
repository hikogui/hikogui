// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_layout_widget.hpp"
#include "../algorithm.hpp"
#include "../alignment.hpp"

namespace tt {

[[nodiscard]] std::pair<int, int> grid_layout_widget::calculate_grid_size(std::vector<cell> const &cells) noexcept
{
    int nr_left = 0;
    int nr_right = 0;
    int nr_top = 0;
    int nr_bottom = 0;

    for (auto &&cell : cells) {
        if (cell.address.row.is_opposite) {
            nr_top = std::max(nr_top, cell.address.row.index + cell.address.row.span);
        } else {
            nr_bottom = std::max(nr_bottom, cell.address.row.index + cell.address.row.span);
        }
        if (cell.address.column.is_opposite) {
            nr_right = std::max(nr_right, cell.address.column.index + cell.address.column.span);
        } else {
            nr_left = std::max(nr_left, cell.address.column.index + cell.address.column.span);
        }
    }

    return {nr_left + nr_right, nr_bottom + nr_top};
}

[[nodiscard]] extent2
grid_layout_widget::calculate_cell_min_size(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    rows.clear();
    columns.clear();

    ttlet[nr_columns, nr_rows] = calculate_grid_size(cells);
    rows.reserve(nr_rows);
    columns.reserve(nr_columns);

    for (auto &&cell : cells) {
        tt_axiom(cell.address.row.is_absolute);
        if (cell.address.row.span == 1) {
            auto index = cell.address.row.begin(nr_rows);

            rows.update(
                index, cell.widget->preferred_size().minimum().height(), cell.widget->height_resistance(), cell.widget->margin());
        }

        tt_axiom(cell.address.column.is_absolute);
        if (cell.address.column.span == 1) {
            auto index = cell.address.column.begin(nr_columns);

            columns.update(
                index,
                cell.widget->preferred_size().minimum().width(),
                cell.widget->width_resistance(),
                cell.widget->margin());
        }
    }

    return {columns.minimum_size(), rows.minimum_size()};
}

std::shared_ptr<widget> grid_layout_widget::add_widget(cell_address address, std::shared_ptr<widget> widget) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    auto tmp = abstract_container_widget::add_widget(std::move(widget));

    if (std::ssize(_children) == 0) {
        // When there are no children, relative addresses need to start at the origin.
        _current_address = "L0T0"_ca;
    } else {
        _current_address *= address;
    }

    _cells.emplace_back(_current_address, tmp);
    return tmp;
}

bool grid_layout_widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (super::update_constraints(display_time_point, need_reconstrain)) {
        _preferred_size = {
            calculate_cell_min_size(_cells, _rows, _columns),
            extent2{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
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
        _columns.set_size(rectangle().width());
        _rows.set_size(rectangle().height());

        for (auto &&cell : _cells) {
            auto &&child = cell.widget;
            ttlet child_rectangle = cell.rectangle(_columns, _rows);
            child->set_layout_parameters_from_parent(child_rectangle);
        }
    }

    abstract_container_widget::update_layout(display_time_point, need_layout);
}

} // namespace tt
