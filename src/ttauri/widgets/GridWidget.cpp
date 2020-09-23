// Copyright 2020 Pokitec
// All rights reserved.

#include "GridWidget.hpp"
#include "../GUI/Window.hpp"
#include "../algorithm.hpp"
#include "../attributes.hpp"

namespace tt {

[[nodiscard]] static std::pair<int, int> calculateGridSize(std::vector<GridWidgetCell> const &cells) noexcept
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

static void calculateCellMinMaxSize(
    std::vector<GridWidgetCell> const &cells,
    std::vector<GridWidgetTick> &rows,
    std::vector<GridWidgetTick> &columns) noexcept
{
    rows.clear();
    columns.clear();

    ttlet[nr_columns, nr_rows] = calculateGridSize(cells);

    rows.resize(nr_rows);
    columns.resize(nr_columns);

    for (auto &&cell : cells) {
        ttlet child_lock = std::scoped_lock(cell.widget->mutex);

        tt_assume(cell.address.row.is_absolute);
        if (cell.address.row.span == 1) {
            auto &row =
                cell.address.row.is_opposite ? rows[std::ssize(rows) - cell.address.row.index - 1] : rows[cell.address.row.index];

            row.minimum = std::max(row.minimum, cell.widget->preferred_size().minimum().height());
            row.maximum = std::min(row.maximum, cell.widget->preferred_size().maximum().height());
            row.base_line = std::max(row.base_line, cell.widget->preferred_base_line());
            tt_assume2(row.minimum <= row.maximum, "Conflicting size of widgets in a row");
        }

        tt_assume(cell.address.column.is_absolute);
        if (cell.address.column.span == 1) {
            auto &column = cell.address.column.is_opposite ? columns[std::ssize(columns) - cell.address.column.index - 1] :
                                                             columns[cell.address.column.index];

            column.minimum = std::max(column.minimum, cell.widget->preferred_size().minimum().width());
            column.maximum = std::min(column.maximum, cell.widget->preferred_size().maximum().width());
            tt_assume2(column.minimum <= column.maximum, "Conflicting size of widgets in a column");
        }
    }
}

[[nodiscard]] std::tuple<ssize_t, float> nonMaximumTicksAndAxisSize(std::vector<GridWidgetTick> const &axis) noexcept
{
    ssize_t count = 0;
    float size = 0.0f;
    for (ttlet &tick : axis) {
        if (tick.size < tick.maximum) {
            ++count;
        }
        size += tick.size;
    }
    return {count, size};
}

static void addToNonMaximumTicks(float size_to_add, std::vector<GridWidgetTick> &axis) noexcept
{
    for (auto &&tick : axis) {
        if (tick.size < tick.maximum) {
            tick.size = std::min(tick.size + size_to_add, tick.maximum);
        }
    }
}

static void setCurrentSizeOfAxis(float total_size, std::vector<GridWidgetTick> &axis) noexcept
{
    // Reset axis.
    for (auto &&tick : axis) {
        tick.size = tick.minimum;
    }

    // Iterate until all of the total_size has been spread over all
    // the ticks off the axis.
    while (true) {
        ttlet[count, size] = nonMaximumTicksAndAxisSize(axis);
        ttlet todo = size - total_size;

        if (count == 0 || todo <= 0.0f) {
            break;
        }

        addToNonMaximumTicks(todo / count, axis);
    }

    // Calculate the offset of each tick
    auto offset = 0.0f;
    for (auto &&tick : axis) {
        tick.offset = offset;
        offset += tick.size;
    }
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

WidgetUpdateResult GridWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ttlet result = ContainerWidget::updateConstraints(); result < WidgetUpdateResult::Self) {
        return result;
    }

    calculateCellMinMaxSize(cells, rows, columns);

    auto row_sum = std::accumulate(rows.cbegin(), rows.cend(), GridWidgetTick{0.0f, 0.0f});
    auto column_sum = std::accumulate(columns.cbegin(), columns.cend(), GridWidgetTick{0.0f, 0.0f});

    _preferred_size = {vec{column_sum.minimum, row_sum.minimum}, vec{column_sum.maximum, row_sum.maximum}};

    return WidgetUpdateResult::Self;
}

WidgetUpdateResult GridWidget::updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());
    forceLayout |= requestLayout.exchange(false);

    if (forceLayout) {
        // We need to pass the sizes for each child before calling the ContainerWidget::updateLayout().
        setCurrentSizeOfAxis(rectangle().width(), columns);
        setCurrentSizeOfAxis(rectangle().height(), rows);

        for(auto &&cell: cells) {
            auto &&child = cell.widget;
            ttlet child_lock = std::scoped_lock(child->mutex);

            ttlet child_rectangle = cell.rectangle(columns, rows);
            ttlet base_line = cell.base_line(rows);

            child->set_window_rectangle(mat::T2{window_rectangle()} * child_rectangle);
        }
    }

    return ContainerWidget::updateLayout(displayTimePoint, forceLayout);
}

} // namespace tt
