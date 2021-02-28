// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_container_widget.hpp"
#include "grid_layout_delegate.hpp"
#include "../GUI/theme.hpp"
#include "../cell_address.hpp"
#include "../flow_layout.hpp"
#include <memory>

namespace tt {

class grid_layout_widget : public abstract_container_widget {
public:
    using super = abstract_container_widget;

    grid_layout_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        std::weak_ptr<grid_layout_delegate> delegate = {}) noexcept :
        abstract_container_widget(window, parent), _delegate(delegate)
    {
    }

    ~grid_layout_widget()
    {
        if (auto delegate_ = _delegate.lock()) {
            delegate_->deinit(*this);
        }
    }

    void init() noexcept override
    {
        if (auto delegate_ = _delegate.lock()) {
            delegate_->init(*this);
        }
    }

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;

    /* Add a widget to the grid.
     */
    std::shared_ptr<widget> add_widget(cell_address address, std::shared_ptr<widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> make_widget_at_address(cell_address address, Args &&...args)
    {
        auto tmp = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        tmp->init();
        return std::static_pointer_cast<T>(add_widget(address, std::move(tmp)));
    }

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, cell_address CellAddress, typename... Args>
    std::shared_ptr<T> make_widget(Args &&...args)
    {
        return make_widget_at_address<T>(CellAddress, std::forward<Args>(args)...);
    }

private:
    struct cell {
        cell_address address;
        std::shared_ptr<tt::widget> widget;

        cell(cell_address address, std::shared_ptr<tt::widget> widget) noexcept : address(address), widget(std::move(widget)) {}

        [[nodiscard]] aarect rectangle(flow_layout const &columns, flow_layout const &rows) const noexcept
        {
            ttlet first_column_nr = address.column.begin(columns.nr_items());
            ttlet last_column_nr = address.column.end(columns.nr_items());
            ttlet first_row_nr = address.row.begin(rows.nr_items());
            ttlet last_row_nr = address.row.end(rows.nr_items());

            ttlet[x, width] = columns.get_offset_and_size(first_column_nr, last_column_nr);
            ttlet[y, height] = rows.get_offset_and_size(first_row_nr, last_row_nr);

            return {x, y, width, height};
        };
    };

    std::vector<cell> _cells;
    cell_address _current_address = "L0T0"_ca;

    std::weak_ptr<grid_layout_delegate> _delegate;

    flow_layout _rows;
    flow_layout _columns;

    [[nodiscard]] static std::pair<int, int> calculate_grid_size(std::vector<cell> const &cells) noexcept;
    [[nodiscard]] static extent2
    calculate_cell_min_size(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept;
};

} // namespace tt
