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
    _layout = {};
    _rows.clear();
    _columns.clear();

    for (ttlet &cell : _cells) {
        ttlet cell_constraints = cell.widget->set_constraints();
        _rows.update(
            cell.row_nr,
            cell_constraints.minimum.height(),
            cell_constraints.preferred.height(),
            cell_constraints.maximum.height(),
            cell.widget->margin());

        _columns.update(
            cell.column_nr,
            cell_constraints.minimum.width(),
            cell_constraints.preferred.width(),
            cell_constraints.maximum.width(),
            cell.widget->margin());
    }

    return _constraints = {
               extent2{_columns.minimum_size(), _rows.minimum_size()},
               extent2{_columns.preferred_size(), _rows.preferred_size()},
               extent2{_columns.maximum_size(), _rows.maximum_size()}};
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
    if (visible) {
        for (ttlet &cell : _cells) {
            cell.widget->draw(context);
        }
    }
}

} // namespace tt
