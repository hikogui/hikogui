// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "abstract_container_widget.hpp"
#include "GridLayoutDelegate.hpp"
#include "../iaarect.hpp"
#include "../GUI/Theme.hpp"
#include "../cell_address.hpp"
#include "../flow_layout.hpp"
#include <memory>

namespace tt {

class GridLayoutWidget : public abstract_container_widget {
public:
    using super = abstract_container_widget;

    GridLayoutWidget(gui_window &window, std::shared_ptr<widget> parent, GridLayoutDelegate *delegate = nullptr) noexcept :
        abstract_container_widget(window, parent), delegate(delegate)
    {
        
    }

    ~GridLayoutWidget()
    {
        if (delegate) {
            delegate->closingWidget();
        }
    }

    void initialize() noexcept override
    {
        if (delegate) {
            delegate->openingWidget(std::static_pointer_cast<GridLayoutWidget>(shared_from_this()));
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
    std::shared_ptr<T> make_widget_at_address(cell_address address, Args &&... args)
    {
        auto tmp = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        tmp->initialize();
        return std::static_pointer_cast<T>(add_widget(address, std::move(tmp)));
    }

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, cell_address CellAddress, typename... Args>
    std::shared_ptr<T> make_widget(Args &&... args)
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
            ttlet first_column_nr = address.column.begin(std::ssize(columns));
            ttlet last_column_nr = address.column.end(std::ssize(columns));
            ttlet first_row_nr = address.row.begin(std::ssize(rows));
            ttlet last_row_nr = address.row.end(std::ssize(rows));

            ttlet[x, width] = columns.get_offset_and_size(first_column_nr, last_column_nr);
            ttlet[y, height] = rows.get_offset_and_size(first_row_nr, last_row_nr);

            return {x, y, width, height};
        };

        [[nodiscard]] relative_base_line base_line(flow_layout const &rows) const noexcept
        {
            ttlet aligned_row_nr = address.row.aligned_to(std::ssize(rows));
            return rows.get_base_line(aligned_row_nr);
        }
    };

    std::vector<cell> cells;
    cell_address current_address = "L0T0"_ca;

    GridLayoutDelegate *delegate = nullptr;

    flow_layout rows;
    flow_layout columns;

    [[nodiscard]] static std::pair<int, int> calculateGridSize(std::vector<cell> const &cells) noexcept;
    [[nodiscard]] static interval_vec2
    calculateCellMinMaxSize(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept;
};

} // namespace tt
