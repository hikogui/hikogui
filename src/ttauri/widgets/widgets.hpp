// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_bool_toggle_button_widget.hpp"
#include "abstract_button_widget.hpp"
#include "abstract_toggle_button_widget.hpp"
#include "boolean_checkbox_widget.hpp"
#include "ButtonWidget.hpp"
#include "checkbox_widget.hpp"
#include "LabelWidget.hpp"
#include "LineInputWidget.hpp"
#include "ScrollViewWidget.hpp"
#include "SelectionWidget.hpp"
#include "toggle_widget.hpp"
#include "overlay_view_widget.hpp"
#include "RadioButtonWidget.hpp"
#include "TabViewWidget.hpp"
#include "ToolbarWidget.hpp"
#include "ToolbarButtonWidget.hpp"
#include "ToolbarTabButtonWidget.hpp"
#include "WindowWidget.hpp"
#include "RowColumnLayoutWidget.hpp"
#include "GridLayoutWidget.hpp"
#include "../GUI/Window.hpp"
#include "../cell_address.hpp"

namespace tt {

/** Add a widget to the main widget of the window.
* The implementation is located here so that widget is a concrete type.
*/
template<typename T, cell_address CellAddress, typename... Args>
T &Window_base::makeWidget(Args &&... args)
{
    tt_assume(widget);
    ttlet lock = std::scoped_lock(widget->mutex);
    return widget->content()->makeWidget<T, CellAddress>(std::forward<Args>(args)...);
}

/** Add a widget to the toolbar of the window.
 * The implementation is located here so that widget is a concrete type.
 */
template<typename T, HorizontalAlignment Alignment, typename... Args>
T &Window_base::makeToolbarWidget(Args &&... args)
{
    tt_assume(widget);
    ttlet lock = std::scoped_lock(widget->mutex);
    return widget->toolbar()->makeWidget<T, Alignment>(std::forward<Args>(args)...);
}

}
