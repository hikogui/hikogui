// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/GUI/Window.hpp"
#include <rhea/constraint.hpp>

namespace TTauri::GUI::Widgets {

class ControlWidget : public Widget {
protected:
    /** The minimum size the widget should be.
    * This value could change based on the content of the widget.
    */
    vec minimumExtent;

    rhea::constraint minimumWidthConstraint;
    rhea::constraint minimumHeightConstraint;

    /** The minimum size the widget should be.
    * This value could change based on the content of the widget.
    */
    vec preferedExtent;

    rhea::constraint preferedWidthConstraint;
    rhea::constraint preferedHeightConstraint;


public:
    ControlWidget(Window &window, Widget *parent, vec defaultExtent) noexcept:
        Widget(window, parent), minimumExtent(defaultExtent), preferedExtent(defaultExtent)
    {
        minimumWidthConstraint = window.addConstraint(box.width >= minimumExtent.width());
        minimumHeightConstraint = window.addConstraint(box.height >= minimumExtent.height());
        //preferedWidthConstraint = window.addConstraint(box.width >= preferedExtent.width());
        //preferedHeightConstraint = window.addConstraint(box.height >= preferedExtent.height());
    }

    ~ControlWidget() {
        window.removeConstraint(minimumWidthConstraint);
        window.removeConstraint(minimumHeightConstraint);
    }

    ControlWidget(const ControlWidget &) = delete;
    ControlWidget &operator=(const ControlWidget &) = delete;
    ControlWidget(ControlWidget &&) = delete;
    ControlWidget &operator=(ControlWidget &&) = delete;

    void setMinimumExtent(vec newMinimumExtent) noexcept {
        if (newMinimumExtent != minimumExtent) {
            minimumExtent = newMinimumExtent;

            minimumWidthConstraint = window.replaceConstraint(
                minimumWidthConstraint,
                box.width >= minimumExtent.width()
            );

            minimumHeightConstraint = window.replaceConstraint(
                minimumHeightConstraint,
                box.height >= minimumExtent.height()
            );
        }
    }

    void setMinimumExtent(float width, float height) noexcept {
        setMinimumExtent(vec{width, height});
    }

};

}