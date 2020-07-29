// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../iaarect.hpp"
#include "../GUI/Theme.hpp"
#include <memory>

namespace tt {

struct GridWidgetCell {
    cell_address address;
    Widget *widget;
    rhea::constraint column_begin_constraint;
    rhea::constraint column_preferred_constraint;
    rhea::constraint column_max_constraint;
    rhea::constraint row_begin_constraint;
    rhea::constraint row_preferred_constraint;
    rhea::constraint row_max_constraint;
    rhea::constraint base_constraint;

    GridWidgetCell(cell_address address, Widget *widget) noexcept :
        address(address), widget(widget) {}
};

class GridWidget : public ContainerWidget {
protected:
    std::vector<rhea::variable> leftGridLines;
    std::vector<rhea::variable> rightGridLines;
    std::vector<rhea::variable> bottomGridLines;
    std::vector<rhea::variable> topGridLines;
    std::vector<rhea::variable> bottomBaseLines;
    std::vector<rhea::variable> topBaseLines;
    std::vector<rhea::constraint> leftGridConstraints;
    std::vector<rhea::constraint> rightGridConstraints;
    std::vector<rhea::constraint> bottomGridConstraints;
    std::vector<rhea::constraint> topGridConstraints;

    std::vector<GridWidgetCell> cells;

    rhea::constraint leftConstraint;
    rhea::constraint rightConstraint;
    rhea::constraint bottomConstraint;
    rhea::constraint topConstraint;
    rhea::constraint rowSplitConstraint;
    rhea::constraint columnSplitConstraint;

    int nrLeftColumns;
    int nrRightColumns;
    int nrTopRows;
    int nrBottomRows;

    void calculateGridSize() noexcept;
    void removeAllConstraints() noexcept;
    void addAllConstraints() noexcept;

    virtual void reconstrain() noexcept override {
        window.stopConstraintSolver();
        ContainerWidget::reconstrain();
        removeAllConstraints();
        calculateGridSize();
        addAllConstraints();
        window.startConstraintSolver();
    }

public:
    GridWidget(Window &window, Widget *parent) noexcept :
        ContainerWidget(window, parent)
    {
        calculateGridSize();
        addAllConstraints();
    }

    /* Add a widget to the grid.
     */
    Widget &addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept override;
};

}
