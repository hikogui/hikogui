


#pragma once

namespace hi { inline namespace v1 {

class icon_delegate {
    virtual ~icon_delegate() = default;

    /** Get the icon.
     *
     * @param sender The widget that wants the icon.
     * @return The icon to display.
     */
    virtual hi::icon icon(widget_intf const &sender) = 0;
};


}}

