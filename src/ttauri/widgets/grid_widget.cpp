// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_widget.hpp"
#include "../algorithm.hpp"
#include "../alignment.hpp"

namespace tt::inline v1 {

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

bool grid_widget::address_in_use(std::size_t column_first, std::size_t row_first, std::size_t column_last, std::size_t row_last) const noexcept
{
    for (ttlet &cell : _cells) {
        if (column_last > cell.column_first and column_first < cell.column_last and row_last > cell.row_last and
            row_first < cell.row_first) {
            return true;
        }
    }
    return false;
}

widget &grid_widget::add_widget(
    std::size_t column_first,
    std::size_t row_first,
    std::size_t column_last,
    std::size_t row_last,
    std::unique_ptr<widget> widget) noexcept
{
    tt_axiom(is_gui_thread());
    tt_assert(
        not address_in_use(column_first, row_first, column_last, row_last),
        "cell ({},{}) of grid_widget is already in use",
        column_first,
        row_first);

    auto &ref = *widget;
    _cells.emplace_back(column_first, row_first, column_last, row_last, std::move(widget));
    request_reconstrain();
    return ref;
}

widget_constraints const &grid_widget::set_constraints() noexcept
{
    _layout = {};
    _rows.clear();
    _columns.clear();

    for (ttlet &cell : _cells) {
        ttlet cell_constraints = cell.widget->set_constraints();
        _rows.add_constraint(
            cell.row_first,
            cell.row_last,
            cell_constraints.minimum.height(),
            cell_constraints.preferred.height(),
            cell_constraints.maximum.height(),
            cell_constraints.margins.top(),
            cell_constraints.margins.bottom());

        _columns.add_constraint(
            cell.column_first,
            cell.column_last,
            cell_constraints.minimum.width(),
            cell_constraints.preferred.width(),
            cell_constraints.maximum.width(),
            cell_constraints.margins.left(),
            cell_constraints.margins.right());
    }
    _rows.commit_constraints();
    _columns.commit_constraints();

    return _constraints = {
       extent2{_columns.minimum(), _rows.minimum()},
       extent2{_columns.preferred(), _rows.preferred()},
       extent2{_columns.maximum(), _rows.maximum()},
       margins{_columns.margin_before(), _rows.margin_after(), _columns.margin_after(), _rows.margin_before()}};
}

void grid_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _columns.layout(layout.width());
        _rows.layout(layout.height());
    }

    for (ttlet &cell : _cells) {
        ttlet child_rectangle = cell.rectangle(_columns, _rows, layout.height());
        cell.widget->set_layout(layout.transform(child_rectangle, 0.0f));
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

hitbox grid_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and enabled) {
        auto r = hitbox{};
        for (ttlet &cell : _cells) {
            r = cell.widget->hitbox_test_from_parent(position, r);
        }
        return r;
    } else {
        return {};
    }
}

} // namespace tt::inline v1
