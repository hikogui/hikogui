// Copyright 2020 Pokitec
// All rights reserved.

#include "GridWidget.hpp"
#include "../GUI/Window.hpp"
#include "../algorithm.hpp"
#include "../alignment.hpp"

namespace tt {

[[nodiscard]] std::pair<int, int> GridWidget::calculateGridSize(std::vector<cell> const &cells) noexcept
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

[[nodiscard]] interval_vec2
GridWidget::calculateCellMinMaxSize(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept
{
    rows.clear();
    columns.clear();

    ttlet[nr_columns, nr_rows] = calculateGridSize(cells);

    for (auto &&cell : cells) {
        ttlet child_lock = std::scoped_lock(cell.widget->mutex);

        tt_assume(cell.address.row.is_absolute);
        if (cell.address.row.span == 1) {
            auto index = cell.address.row.begin(nr_rows);

            rows.update(
                index,
                cell.widget->preferred_size().height(),
                cell.widget->height_resistance(),
                cell.widget->margin(),
                cell.widget->preferred_base_line());
        }

        tt_assume(cell.address.column.is_absolute);
        if (cell.address.column.span == 1) {
            auto index = cell.address.column.begin(nr_columns);

            columns.update(
                index,
                cell.widget->preferred_size().width(),
                cell.widget->width_resistance(),
                cell.widget->margin(),
                relative_base_line{});
        }
    }

    return {columns.extent(), rows.extent()};
}

Widget &GridWidget::addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept
{
    auto lock = std::scoped_lock(mutex);

    if (std::ssize(children) == 0) {
        // When there are no children, relative addresses need to start at the origin.
        current_address = "L0T0"_ca;
    } else {
        current_address *= address;
    }

    auto &widget = ContainerWidget::addWidget(std::move(childWidget));
    cells.emplace_back(current_address, &widget);
    return widget;
}

bool GridWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ContainerWidget::updateConstraints()) {
        _preferred_size = calculateCellMinMaxSize(cells, rows, columns);
        return true;
    } else {
        return false;
    }
}

bool GridWidget::updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    need_layout |= std::exchange(requestLayout, false);
    if (need_layout) {
        columns.flow(rectangle().width());
        rows.flow(rectangle().height());

        for (auto &&cell : cells) {
            auto &&child = cell.widget;
            ttlet child_lock = std::scoped_lock(child->mutex);

            ttlet child_rectangle = cell.rectangle(columns, rows);
            ttlet child_base_line = cell.base_line(rows);

            ttlet child_window_rectangle = mat::T2{_window_rectangle} * child_rectangle;
            ttlet child_base_line_position =
                child_base_line.position(child_window_rectangle.bottom(), child_window_rectangle.top());

            child->set_layout_parameters(child_window_rectangle, _window_clipping_rectangle, child_base_line_position);
        }
    }

    return ContainerWidget::updateLayout(display_time_point, need_layout);
}

} // namespace tt
