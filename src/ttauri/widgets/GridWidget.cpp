// Copyright 2020 Pokitec
// All rights reserved.

#include "GridWidget.hpp"
#include "../GUI/Window.hpp"
#include "../algorithm.hpp"

namespace tt {

Widget &GridWidget::addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept
{
    auto lock = std::scoped_lock(mutex);

    auto &widget = ContainerWidget::addWidget(address, std::move(childWidget));
    cells.emplace_back(current_address, &widget);
    return widget;
}

void GridWidget::removeAllConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    for (auto &&cell: cells) {
        window.removeConstraint(cell.column_begin_constraint);
        window.removeConstraint(cell.column_preferred_constraint);
        window.removeConstraint(cell.column_max_constraint);
        window.removeConstraint(cell.row_begin_constraint);
        window.removeConstraint(cell.row_preferred_constraint);
        window.removeConstraint(cell.row_max_constraint);
        window.removeConstraint(cell.base_constraint);
    }

    window.removeConstraint(leftConstraint);
    window.removeConstraint(rightConstraint);
    window.removeConstraint(bottomConstraint);
    window.removeConstraint(topConstraint);
    window.removeConstraint(columnSplitConstraint);
    window.removeConstraint(rowSplitConstraint);

    for (auto &&constraint: leftGridConstraints) {
        window.removeConstraint(constraint);
    }
    for (auto &&constraint: rightGridConstraints) {
        window.removeConstraint(constraint);
    }
    for (auto &&constraint: bottomGridConstraints) {
        window.removeConstraint(constraint);
    }
    for (auto &&constraint: topGridConstraints) {
        window.removeConstraint(constraint);
    }

    leftGridConstraints.clear();
    rightGridConstraints.clear();
    bottomGridConstraints.clear();
    topGridConstraints.clear();

    leftGridLines.clear();
    rightGridLines.clear();
    bottomGridLines.clear();
    topGridLines.clear();
}

void GridWidget::calculateGridSize() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto addresses = transform<std::vector<cell_address>>(cells, [](auto &item) { return item.address; });

    std::tie(nrLeftColumns, nrRightColumns, nrBottomRows, nrTopRows) = cell_address_max(addresses.cbegin(), addresses.cend());
}

void GridWidget::addAllConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    while (nonstd::ssize(leftGridLines) < nrLeftColumns + 1) {
        leftGridLines.emplace_back();
    }
    while (nonstd::ssize(rightGridLines) < nrRightColumns + 1) {
        rightGridLines.emplace_back();
    }
    while (nonstd::ssize(bottomGridLines) < nrBottomRows + 1) {
        bottomGridLines.emplace_back();
    }
    while (nonstd::ssize(topGridLines) < nrTopRows + 1) {
        topGridLines.emplace_back();
    }
    while (nonstd::ssize(bottomBaseLines) < nrBottomRows) {
        bottomBaseLines.emplace_back();
    }
    while (nonstd::ssize(topBaseLines) < nrTopRows) {
        topBaseLines.emplace_back();
    }

    for (int i = 0; i != nrLeftColumns; ++i) {
        leftGridConstraints.push_back(window.addConstraint(leftGridLines[i] <= leftGridLines[i+1]));
    }
    for (int i = 0; i != nrRightColumns; ++i) {
        rightGridConstraints.push_back(window.addConstraint(rightGridLines[i] >= rightGridLines[i+1]));
    }
    for (int i = 0; i != nrBottomRows; ++i) {
        bottomGridConstraints.push_back(window.addConstraint(bottomGridLines[i] <= bottomGridLines[i+1]));
    }
    for (int i = 0; i != nrTopRows; ++i) {
        topGridConstraints.push_back(window.addConstraint(topGridLines[i] >= topGridLines[i+1]));
    }

    leftConstraint = window.addConstraint(leftGridLines.front() == left - Theme::margin * 0.5f);
    rightConstraint = window.addConstraint(rightGridLines.front() == right + Theme::margin * 0.5f);
    bottomConstraint = window.addConstraint(bottomGridLines.front() == bottom - Theme::margin * 0.5f);
    topConstraint = window.addConstraint(topGridLines.front() == top + Theme::margin * 0.5f);
    columnSplitConstraint = window.addConstraint(leftGridLines.back() <= rightGridLines.back());
    rowSplitConstraint = window.addConstraint(bottomGridLines.back() <= topGridLines.back());

    for (auto &&cell: cells) {
        ttlet xbegin = begin<false>(cell.address);
        ttlet xend = end<false>(cell.address);
        ttlet ybegin = begin<true>(cell.address);
        ttlet yend = end<true>(cell.address);
        ttlet ybase = ybegin + get_alignment<true>(cell.address);

        if (is_opposite<false>(cell.address)) {
            cell.column_begin_constraint = window.addConstraint(cell.widget->right + Theme::margin * 0.5f == rightGridLines[xbegin]);
            cell.column_preferred_constraint = window.addConstraint(cell.widget->left - Theme::margin * 0.5f == rightGridLines[xend], rhea::strength::medium());
            cell.column_max_constraint = window.addConstraint(cell.widget->left - Theme::margin * 0.5f >= rightGridLines[xend]);
        } else {
            cell.column_begin_constraint = window.addConstraint(cell.widget->left - Theme::margin * 0.5f == leftGridLines[xbegin]);
            cell.column_preferred_constraint = window.addConstraint(cell.widget->right + Theme::margin * 0.5f == leftGridLines[xend], rhea::strength::medium());
            cell.column_max_constraint = window.addConstraint(cell.widget->right + Theme::margin * 0.5f <= leftGridLines[xend]);
        }

        if (is_opposite<true>(cell.address)) {
            cell.row_begin_constraint = window.addConstraint(cell.widget->top + Theme::margin * 0.5f == topGridLines[ybegin]);
            cell.row_preferred_constraint = window.addConstraint(cell.widget->bottom - Theme::margin * 0.5f == topGridLines[yend], rhea::strength::medium());
            cell.row_max_constraint = window.addConstraint(cell.widget->bottom - Theme::margin * 0.5f >= topGridLines[yend]);

            cell.base_constraint = window.addConstraint(cell.widget->base == topBaseLines[ybase]);
        } else {
            cell.row_begin_constraint = window.addConstraint(cell.widget->bottom - Theme::margin * 0.5f == bottomGridLines[ybegin]);
            cell.row_preferred_constraint = window.addConstraint(cell.widget->top + Theme::margin * 0.5f == bottomGridLines[yend], rhea::strength::medium());
            cell.row_max_constraint = window.addConstraint(cell.widget->top + Theme::margin * 0.5f <= bottomGridLines[yend]);

            cell.base_constraint = window.addConstraint(cell.widget->base == bottomBaseLines[ybase]);
        }
    }
}

WidgetUpdateResult GridWidget::updateConstraints() noexcept {
    tt_assume(mutex.is_locked_by_current_thread());

    if (ttlet result = ContainerWidget::updateConstraints(); result < WidgetUpdateResult::Self) {
        return result;
    }

    window.stopConstraintSolver();
    removeAllConstraints();
    calculateGridSize();
    addAllConstraints();
    window.startConstraintSolver();
    return WidgetUpdateResult::Self;
}

}
