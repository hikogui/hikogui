// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_widget.hpp"
#include "../algorithm.hpp"
#include "../alignment.hpp"

namespace tt {

grid_widget::grid_widget(
    gui_window &window,
    widget *parent,
    std::weak_ptr<delegate_type> delegate) noexcept :
    widget(window, parent), _delegate(std::move(delegate))
{
    tt_axiom(is_gui_thread());

    if (parent) {
        semantic_layer = parent->semantic_layer;
    }
}

void grid_widget::init() noexcept
{
    if (auto delegate = _delegate.lock()) {
        delegate->init(*this);
    }
}

void grid_widget::deinit() noexcept
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

[[nodiscard]] float grid_widget::margin() const noexcept
{
    return 0.0f;
}

[[nodiscard]] std::pair<size_t, size_t> grid_widget::calculate_grid_size(std::vector<cell> const &cells) noexcept
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
grid_widget::calculate_size(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept
{
    rows.clear();
    columns.clear();

    ttlet[nr_columns, nr_rows] = calculate_grid_size(cells);
    rows.reserve(nr_rows);
    columns.reserve(nr_columns);

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

bool grid_widget::address_in_use(size_t column_nr, size_t row_nr) const noexcept
{
    for (ttlet &cell : _cells) {
        if (cell.column_nr == column_nr && cell.row_nr == row_nr) {
            return true;
        }
    }
    return false;
}

widget &grid_widget::add_widget(size_t column_nr, size_t row_nr, std::unique_ptr<widget> widget) noexcept
{
    tt_axiom(is_gui_thread());
    tt_assert(!address_in_use(column_nr, row_nr), "cell ({},{}) of grid_widget is already in use", column_nr, row_nr);

    auto &tmp = super::add_widget(std::move(widget));
    _cells.emplace_back(column_nr, row_nr, &tmp);
    return tmp;
}

bool grid_widget::constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        std::tie(_minimum_size, _preferred_size, _maximum_size) = calculate_size(_cells, _rows, _columns);
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

void grid_widget::layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        _columns.set_size(width());
        _rows.set_size(height());

        for (auto &&cell : _cells) {
            auto &&child = cell.widget;
            ttlet child_rectangle = cell.rectangle(_columns, _rows, height());
            child->set_layout_parameters_from_parent(child_rectangle);
        }
    }

    super::layout(display_time_point, need_layout);
}

} // namespace tt
