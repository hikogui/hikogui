// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../iaarect.hpp"
#include "../GUI/Theme.hpp"
#include <memory>

namespace tt {

struct GridWidgetCell {
    WidgetPosition position;
    Widget *widget;
    rhea::constraint left_constraint;
    rhea::constraint right_constraint;
    rhea::constraint top_constraint;
    rhea::constraint bottom_constraint;

    GridWidgetCell(WidgetPosition position, Widget *widget) noexcept :
        position(position), widget(widget) {}

    [[nodiscard]] int firstColumn(int width) noexcept {
        return position.firstColumn(width);
    }
    [[nodiscard]] int lastColumn(int width) noexcept {
        return position.lastColumn(width);
    }
    [[nodiscard]] int firstRow(int height) noexcept {
        return position.firstRow(height);
    }
    [[nodiscard]] int lastRow(int height) noexcept {
        return position.lastRow(height);
    }
};

class GridWidget : public ContainerWidget {
protected:
    std::vector<rhea::variable> colGridLines;
    std::vector<rhea::variable> rowGridLines;
    std::vector<GridWidgetCell> cells;

    int nrColumns;
    int nrRows;
    int nrLeftColumns;
    int nrRightColumns;
    int nrTopRows;
    int nrBottomRows;

    void calculateGridSize() noexcept;
    void removeAllConstraints() noexcept;
    void addAllConstraints() noexcept;

public:
    GridWidget(Window &window, Widget *parent) noexcept :
        ContainerWidget(window, parent)
    {
        calculateGridSize();
        addAllConstraints();
    }

    WidgetPosition nextPosition() noexcept override;

    Widget &addWidget(WidgetPosition position, std::unique_ptr<Widget> childWidget) noexcept override;
};

}
