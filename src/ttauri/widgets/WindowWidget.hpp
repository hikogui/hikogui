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

    /** The minimum extend that the contained widgets allow.
     */
    [[nodiscard]] vec minimumExtent() const noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        return _minimumExtent;
    }

    /** The maximum extend that the contained widgets allow.
     */
    [[nodiscard]] vec maximumExtent() const noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        return _maximumExtent;
    }

    /** The preferred extend that the contained widgets would like.
     */
    [[nodiscard]] vec preferredExtent() const noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        return _preferredExtent;
    }

    /** The window extend within range of how the contained widgets would like to be laid out.
     */
    [[nodiscard]] vec windowExtent() const noexcept {
        tt_assume(mutex.is_locked_by_current_thread());
        return _windowExtent;
    }

    /** Set the size of the window.
     * When the window changes shape this should be communicated through this function
     * to the WindowWidget.
     *
     * In turn the WindowWidget will reconstrain the contained widgets on the next
     * frame. The reconstraining may change the result of `windowExtent()`.
     */
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
