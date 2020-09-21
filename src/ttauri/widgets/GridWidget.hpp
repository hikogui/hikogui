// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../iaarect.hpp"
#include "../GUI/Theme.hpp"
#include <memory>

namespace tt {

struct GridWidgetTick {
    float minimum = 0.0f;
    float maximum = std::numeric_limits<float>::infinity();
    float size = 0.0f;
    float offset = 0.0f;
    base_line _base_line = base_line{VerticalAlignment::Middle, 0.0f, -std::numeric_limits<float>::infinity()};

    [[nodiscard]] friend GridWidgetTick operator+(GridWidgetTick const &lhs, GridWidgetTick const &rhs) noexcept
    {
        GridWidgetTick r;
        r.minimum = lhs.minimum + rhs.minimum;
        r.maximum = lhs.maximum + rhs.maximum;
        r.size = 0.0f;
        r.offset = 0.0f;
        r._base_line = std::max(lhs._base_line, rhs._base_line);
        return r;
    }
};

struct GridWidgetCell {
    cell_address address;
    Widget *widget;

    GridWidgetCell(cell_address address, Widget *widget) noexcept : address(address), widget(widget) {}

    [[nodiscard]] aarect
    rectangle(std::vector<GridWidgetTick> const &columns, std::vector<GridWidgetTick> const &rows) const noexcept
    {
        ttlet first_column_nr = address.column.begin(std::ssize(columns));
        ttlet last_column_nr = address.column.end(std::ssize(columns));
        ttlet first_row_nr = address.row.begin(std::ssize(rows));
        ttlet last_row_nr = address.row.end(std::ssize(rows));

        ttlet x = columns[first_column_nr].offset;
        ttlet y = rows[first_row_nr].offset;

        auto width = 0.0f;
        for (auto i = first_column_nr; i != last_column_nr; ++i) {
            width += columns[i].size;
        }

        auto height = 0.0f;
        for (auto i = first_row_nr; i != last_row_nr; ++i) {
            height += rows[i].size;
        }
        return {x, y, width, height};
    };

    [[nodiscard]] base_line base_line(std::vector<GridWidgetTick> const &rows) const noexcept
    {
        ttlet aligned_row_nr = address.row.aligned_to(std::ssize(rows));
        return rows[aligned_row_nr]._base_line;
    }
};

class GridWidget : public ContainerWidget {
public:
    GridWidget(Window &window, Widget *parent, ContainerWidgetDelegate *delegate = nullptr) noexcept :
        ContainerWidget(window, parent, delegate)
    {
    }

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override;
    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override;

    /* Add a widget to the grid.
     */
    Widget &addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept override;

protected:
    std::vector<GridWidgetCell> cells;

private:
    std::vector<GridWidgetTick> rows;
    std::vector<GridWidgetTick> columns;
};

} // namespace tt
