// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_bool_toggle_button_widget.hpp"
#include "abstract_button_widget.hpp"
#include "abstract_toggle_button_widget.hpp"
#include "boolean_checkbox_widget.hpp"
#include "button_widget.hpp"
#include "checkbox_widget.hpp"
#include "LabelWidget.hpp"
#include "LineInputWidget.hpp"
#include "ScrollViewWidget.hpp"
#include "SelectionWidget.hpp"
#include "toggle_widget.hpp"
#include "overlay_view_widget.hpp"
#include "radio_button_widget.hpp"
#include "tab_view_widget.hpp"
#include "toolbar_widget.hpp"
#include "toolbar_button_widget.hpp"
#include "toolbar_tab_button_widget.hpp"
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
std::shared_ptr<T> Window_base::make_widget(Args &&... args)
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);
    tt_assume(widget);
    return widget->content()->make_widget<T, CellAddress>(std::forward<Args>(args)...);
}

/** Add a widget to the toolbar of the window.
 * The implementation is located here so that widget is a concrete type.
 */
template<typename T, HorizontalAlignment Alignment, typename... Args>
std::shared_ptr<T> Window_base::make_toolbar_widget(Args &&... args)
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);
    tt_assume(widget);
    return widget->toolbar()->make_widget<T, Alignment>(std::forward<Args>(args)...);
}

}
