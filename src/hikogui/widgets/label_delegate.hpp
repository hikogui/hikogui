


#pragma once

#include "icon_delegate.hpp"
#include "text_delegate.hpp"

namespace hi { inline namespace v1 {

class label_delegate : public icon_delegate, public text_delegate {
    virtual ~label_delegate() = default;

    /** Get the label.
     *
     * @param sender The widget that wants the label.
     * @return The label to display.
     */
    virtual hi::label label(widget_intf const &sender) = 0;
};


}}

