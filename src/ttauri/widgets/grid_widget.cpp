// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_widget.hpp"
#include "../algorithm.hpp"
#include "../alignment.hpp"

namespace tt {

grid_widget::grid_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    widget(window, parent), _delegate(std::move(delegate))
{
    tt_axiom(is_gui_thread());

    if (parent) {
        semantic_layer = parent->semantic_layer;
    }
    if (auto d = _delegate.lock()) {
        d->init(*this);
    }
}

grid_widget::~grid_widget()
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
            cell.widget->constraints().min.height(),
            cell.widget->constraints().pref.height(),
            cell.widget->constraints().max.height(),
            cell.widget->margin());

        columns.update(
            cell.column_nr,
            cell.widget->constraints().min.width(),
            cell.widget->constraints().pref.width(),
            cell.widget->constraints().max.width(),
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

    auto &ref = *widget;
    _cells.emplace_back(column_nr, row_nr, std::move(widget));
    window.request_reconstrain();
    return ref;
}

widget_constraints const &grid_widget::set_constraints() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    for (ttlet &cell : _cells) {
        cell.widget->set_constraints();
    }

    std::tie(_constraints.min, _constraints.pref, _constraints.max) = calculate_size(_cells, _rows, _columns);
    tt_axiom(_constraints.min <= _constraints.pref && _constraints.pref <= _constraints.max);
    return _constraints;
}

void grid_widget::set_layout(widget_layout const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            _columns.set_size(layout().width());
            _rows.set_size(layout().height());
        }

        for (ttlet &cell : _cells) {
            cell.widget->set_layout(cell.rectangle(_columns, _rows, layout().height()) * context);
        }
    }
}

void grid_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        for (ttlet &cell : _cells) {
            cell.widget->draw(context);
        }
    }
}

} // namespace tt
