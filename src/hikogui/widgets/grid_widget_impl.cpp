// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grid_widget.hpp"
#include "../algorithm.hpp"
#include "../geometry/module.hpp"
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

widget& grid_widget::add_widget(
    std::size_t first_column,
    std::size_t first_row,
    std::size_t last_column,
    std::size_t last_row,
    std::unique_ptr<widget> widget) noexcept
{
    hi_axiom(loop::main().on_thread());
    hi_axiom(first_column < last_column);
    hi_axiom(first_row < last_row);

    if (_grid.cell_in_use(first_column, first_row, last_column, last_row)) {
        hi_log_fatal("cell ({},{}) of grid_widget is already in use", first_column, first_row);
    }

    auto& ref = *widget;
    _grid.add_cell(first_column, first_row, last_column, last_row, std::move(widget));
    hi_log_info("grid_widget::add_widget({}, {}, {}, {})", first_column, first_row, last_column, last_row);

    ++global_counter<"grid_widget:add_widget:constrain">;
    process_event({gui_event_type::window_reconstrain});
    return ref;
}

[[nodiscard]] box_constraints grid_widget::update_constraints() noexcept
{
    _layout = {};

    for (auto& cell : _grid) {
        cell.set_constraints(cell.value->update_constraints());
    }

    return _grid.constraints(os_settings::left_to_right());
}

void grid_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        _grid.set_layout(context.shape, theme().baseline_adjustment());
    }

    for (hilet& cell : _grid) {
        cell.value->set_layout(context.transform(cell.shape, 0.0f));
    }
}

void grid_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible) {
        for (hilet& cell : _grid) {
            cell.value->draw(context);
        }
    }
}

hitbox grid_widget::hitbox_test(point2i position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial) {
        auto r = hitbox{};
        for (hilet& cell : _grid) {
            r = cell.value->hitbox_test_from_parent(position, r);
        }
        return r;
    } else {
        return {};
    }
}

} // namespace hi::inline v1
