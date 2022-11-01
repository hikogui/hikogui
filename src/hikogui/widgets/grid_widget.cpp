// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_widget.hpp"
#include "../algorithm.hpp"
#include "../alignment.hpp"
#include "../log.hpp"

namespace hi::inline v1 {

grid_widget::grid_widget(widget *parent) noexcept : widget(parent)
{
    hi_axiom(loop::main().on_thread());

    if (parent) {
        semantic_layer = parent->semantic_layer;
    }
}

grid_widget::~grid_widget() {}

bool grid_widget::address_in_use(std::size_t column_first, std::size_t row_first, std::size_t column_last, std::size_t row_last)
    const noexcept
{
    for (hilet& cell : _cells) {
        if (column_last > cell.column_first and column_first < cell.column_last and row_last > cell.row_last and
            row_first < cell.row_first) {
            return true;
        }
    }
    return false;
}

widget& grid_widget::add_widget(
    std::size_t column_first,
    std::size_t row_first,
    std::size_t column_last,
    std::size_t row_last,
    std::unique_ptr<widget> widget) noexcept
{
    hi_axiom(loop::main().on_thread());
    if (address_in_use(column_first, row_first, column_last, row_last)) {
        hi_log_fatal("cell ({},{}) of grid_widget is already in use", column_first, row_first);
    }

    auto& ref = *widget;
    _cells.emplace_back(column_first, row_first, column_last, row_last, std::move(widget));
    hi_log_info("grid_widget::add_widget({}, {}, {}, {})", column_first, row_first, column_last, row_last);
    ++global_counter<"grid_widget:add_widget:constrain">;
    process_event({gui_event_type::window_reconstrain});
    return ref;
}

widget_constraints const& grid_widget::set_constraints(set_constraints_context const& context) noexcept
{
    _layout = {};
    _rows.clear();
    _columns.clear();

    for (hilet& cell : _cells) {
        hilet cell_constraints = cell.widget->set_constraints(context);
        _rows.add_constraint(
            cell.row_first,
            cell.row_last,
            cell_constraints.minimum.height(),
            cell_constraints.preferred.height(),
            cell_constraints.maximum.height(),
            cell_constraints.margins.top(),
            cell_constraints.margins.bottom(),
            cell_constraints.baseline);

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

void grid_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        _columns.layout(context.width());
        _rows.layout(context.height());
    }

    for (hilet& cell : _cells) {
        hilet child_rectangle = cell.rectangle(_columns, _rows, context.size, context.left_to_right());
        hilet child_baseline = cell.baseline(_rows);
        cell.widget->set_layout(context.transform(child_rectangle, 0.0f, child_baseline));
    }
}

void grid_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible) {
        for (hilet& cell : _cells) {
            cell.widget->draw(context);
        }
    }
}

hitbox grid_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial) {
        auto r = hitbox{};
        for (hilet& cell : _cells) {
            r = cell.widget->hitbox_test_from_parent(position, r);
        }
        return r;
    } else {
        return {};
    }
}

} // namespace hi::inline v1
