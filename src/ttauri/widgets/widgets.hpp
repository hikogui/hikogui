// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_bool_toggle_button_widget.hpp"
#include "abstract_button_widget.hpp"
#include "abstract_toggle_button_widget.hpp"
#include "boolean_checkbox_widget.hpp"
#include "button_widget.hpp"
#include "checkbox_widget.hpp"
#include "label_widget.hpp"
#include "text_field_widget.hpp"
#include "scroll_view_widget.hpp"
#include "selection_widget.hpp"
#include "toggle_widget.hpp"
#include "overlay_view_widget.hpp"
#include "radio_button_widget.hpp"
#include "tab_view_widget.hpp"
#include "toolbar_widget.hpp"
#include "menu_item_widget.hpp"
#include "toolbar_tab_button_widget.hpp"
#include "window_widget.hpp"
#include "row_column_layout_widget.hpp"
#include "grid_layout_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../cell_address.hpp"

namespace tt {

/** Add a widget to the main widget of the window.
* The implementation is located here so that widget is a concrete type.
*/
template<typename T, cell_address CellAddress, typename... Args>
std::shared_ptr<T> gui_window::make_widget(Args &&... args)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    tt_axiom(widget);
    return widget->content()->make_widget<T, CellAddress>(std::forward<Args>(args)...);
}

/** Add a widget to the toolbar of the window.
 * The implementation is located here so that widget is a concrete type.
 */
template<typename T, horizontal_alignment Alignment, typename... Args>
std::shared_ptr<T> gui_window::make_toolbar_widget(Args &&... args)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    tt_axiom(widget);
    return widget->toolbar()->make_widget<T, Alignment>(std::forward<Args>(args)...);
}

}
