// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../cells/Label.hpp"

namespace tt {

class ToolbarWidget;

class WindowWidget : public ContainerWidget {
    Label title;

    vec _minimumExtent;
    vec _maximumExtent;
    vec _preferredExtent;
    vec _windowExtent;

    rhea::constraint leftConstraint;
    rhea::constraint bottomConstraint;
    rhea::constraint widthConstraint;
    rhea::constraint heightConstraint;
public:
    ContainerWidget *content = nullptr;
    ToolbarWidget *toolbar = nullptr;

    WindowWidget(Window &window, Label title) noexcept;
    ~WindowWidget();

    [[nodiscard]] vec minimumExtent() const noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        return _minimumExtent;
    }

    [[nodiscard]] vec maximumExtent() const noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        return _maximumExtent;
    }

    [[nodiscard]] vec preferredExtent() const noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        return _preferredExtent;
    }

    [[nodiscard]] vec windowExtent() const noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        return _windowExtent;
    }

    void setWindowExtent(vec newExtent) noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        if (_windowExtent != newExtent) {
            _windowExtent = newExtent;
            requestConstraint = true;
        }
    }

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;
};

}
