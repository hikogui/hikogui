// Copyright 2020 Pokitec
// All rights reserved.

#include "GridWidget.hpp"
#include "../GUI/Window.hpp"

namespace tt {

Widget &GridWidget::addWidget(WidgetPosition position, std::unique_ptr<Widget> childWidget) noexcept
{
    window.stopConstraintSolver();
    removeAllConstraints();

    auto &widget = ContainerWidget::addWidget(position, std::move(childWidget));

    cells.emplace_back(position, &widget);

    calculateGridSize();
    addAllConstraints();
    window.startConstraintSolver();

    return widget;
}

void GridWidget::removeAllConstraints() noexcept
{
    [[maybe_unused]] ttlet lock = std::scoped_lock(mutex);

    for (auto &&cell: cells) {
        window.removeConstraint(cell.left_constraint);
        window.removeConstraint(cell.right_constraint);
        window.removeConstraint(cell.top_constraint);
        window.removeConstraint(cell.bottom_constraint);
    }
}

void GridWidget::calculateGridSize() noexcept
{
    [[maybe_unused]] ttlet lock = std::scoped_lock(mutex);

    nrLeftColumns = 0;
    nrRightColumns = 0;
    nrTopRows = 0;
    nrBottomRows = 0;

    for (ttlet &cell: cells) {
        if (cell.position.col >= 0) {
            nrLeftColumns = std::max(nrLeftColumns, cell.position.col + cell.position.colspan);
        } else {
            nrRightColumns = std::max(nrRightColumns, -cell.position.col - 1 + cell.position.colspan);
        }
        if (cell.position.row >= 0) {
            nrBottomRows = std::max(nrBottomRows, cell.position.row + cell.position.rowspan);
        } else {
            nrTopRows = std::max(nrTopRows, -cell.position.row - 1 + cell.position.rowspan);
        }
    }

    nrColumns = nrLeftColumns + nrRightColumns;
    nrRows = nrBottomRows + nrTopRows;
}

WidgetPosition GridWidget::nextPosition() noexcept
{
    return {0, -nrTopRows - 1, 1, 1};
}

void GridWidget::addAllConstraints() noexcept
{
    [[maybe_unused]] ttlet lock = std::scoped_lock(mutex);

    while (nonstd::ssize(colGridLines) < nrColumns - 1) {
        colGridLines.emplace_back();
    }
    while (nonstd::ssize(rowGridLines) < nrRows - 1) {
        rowGridLines.emplace_back();
    }

    for (auto &&cell: cells) {
        ttlet x1 = cell.firstColumn(nrColumns);
        ttlet x2 = cell.lastColumn(nrColumns);
        ttlet y1 = cell.firstRow(nrRows);
        ttlet y2 = cell.lastRow(nrRows);

        if (x1 == 0) {
            cell.left_constraint = window.addConstraint(cell.widget->left == left, rhea::strength::strong());
        } else {
            cell.left_constraint = window.addConstraint(cell.widget->left == colGridLines[x1 - 1] + Theme::margin * 0.5f, rhea::strength::strong());
        }
        if (x2 == nrColumns - 1) {
            cell.right_constraint = window.addConstraint(cell.widget->right == right, rhea::strength::strong());
        } else {
            cell.right_constraint = window.addConstraint(cell.widget->right == colGridLines[x2] - Theme::margin * 0.5f, rhea::strength::strong());
        }

        if (y1 == 0) {
            cell.bottom_constraint = window.addConstraint(cell.widget->bottom == bottom, rhea::strength::strong());
        } else {
            cell.bottom_constraint = window.addConstraint(cell.widget->bottom == rowGridLines[y1 - 1] + Theme::margin * 0.5f, rhea::strength::strong());
        }
        if (y2 == nrRows - 1) {
            cell.top_constraint = window.addConstraint(cell.widget->top == top, rhea::strength::strong());
        } else {
            cell.top_constraint = window.addConstraint(cell.widget->top == rowGridLines[y2] - Theme::margin * 0.5f, rhea::strength::strong());
        }
    }
}

}
