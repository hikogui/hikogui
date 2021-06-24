// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "grid_layout_delegate.hpp"
#include "../geometry/spread_sheet_address.hpp"
#include "../GUI/theme.hpp"
#include "../flow_layout.hpp"
#include <memory>

namespace tt {

class grid_layout_widget : public widget {
public:
    using super = widget;
    using delegate_type = grid_layout_delegate;

    grid_layout_widget(
        gui_window &window,
        widget *parent,
        std::shared_ptr<delegate_type> delegate = std::make_shared<delegate_type>()) noexcept;

    void init() noexcept override;
    void deinit() noexcept override;

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;

    /* Add a widget to the grid.
     */
    widget &add_widget(size_t column_nr, size_t row_nr, std::unique_ptr<widget> child_widget) noexcept;

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, typename... Args>
    T &make_widget(size_t column_nr, size_t row_nr, Args &&...args)
    {
        auto tmp = std::make_unique<T>(window, this, std::forward<Args>(args)...);
        tmp->init();
        return static_cast<T &>(add_widget(column_nr, row_nr, std::move(tmp)));
    }

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, typename... Args>
    T &make_widget(std::string_view address, Args &&...args)
    {
        ttlet [column_nr, row_nr] = parse_spread_sheet_address(address);
        return make_widget<T>(column_nr, row_nr, std::forward<Args>(args)...);
    }

private:
    struct cell {
        size_t column_nr;
        size_t row_nr;
        tt::widget *widget;

        cell(size_t column_nr, size_t row_nr, tt::widget *widget) noexcept :
            column_nr(column_nr), row_nr(row_nr), widget(widget)
        {
        }

        [[nodiscard]] aarectangle rectangle(flow_layout const &columns, flow_layout const &rows, float container_height) const noexcept
        {
            ttlet[x, width] = columns.get_offset_and_size(column_nr);
            ttlet[y, height] = rows.get_offset_and_size(row_nr);

            return {x, container_height - y - height, width, height};
        };
    };

    std::vector<cell> _cells;

    flow_layout _rows;
    flow_layout _columns;

    std::shared_ptr<grid_layout_delegate> _delegate;

    [[nodiscard]] static std::pair<size_t, size_t> calculate_grid_size(std::vector<cell> const &cells) noexcept;
    [[nodiscard]] static std::tuple<extent2, extent2, extent2>
    calculate_size(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept;
    [[nodiscard]] bool address_in_use(size_t column_nr, size_t row_nr) const noexcept;
};

} // namespace tt
