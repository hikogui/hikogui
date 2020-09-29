// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "GridWidgetDelegate.hpp"
#include "../iaarect.hpp"
#include "../GUI/Theme.hpp"
#include "../cell_address.hpp"
#include "../flow_layout.hpp"
#include <memory>

namespace tt {

struct GridWidgetCell {
    cell_address address;
    Widget *widget;

    GridWidgetCell(cell_address address, Widget *widget) noexcept : address(address), widget(widget) {}

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

class GridWidget : public ContainerWidget {
public:
    GridWidget(Window &window, Widget *parent, GridWidgetDelegate *delegate = nullptr) noexcept :
        ContainerWidget(window, parent), delegate(delegate)
    {
        if (delegate) {
            delegate->openingWidget(*this);
        }
    }

    ~GridWidget()
    {
        if (delegate) {
            delegate->closingWidget(*this);
        }
    }

    [[nodiscard]] bool updateConstraints() noexcept override;
    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;

    /* Add a widget to the grid.
     */
    Widget &addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, typename... Args>
    T &makeWidgetAtAddress(cell_address address, Args &&... args)
    {
        return static_cast<T &>(addWidget(address, std::make_unique<T>(window, this, std::forward<Args>(args)...)));
    }

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, cell_address CellAddress, typename... Args>
    T &makeWidget(Args &&... args)
    {
        return makeWidgetAtAddress<T>(CellAddress, std::forward<Args>(args)...);
    }

protected:
    std::vector<GridWidgetCell> cells;
    cell_address current_address = "L0T0"_ca;

    GridWidgetDelegate *delegate = nullptr;

private:
    flow_layout rows;
    flow_layout columns;
};

} // namespace tt
