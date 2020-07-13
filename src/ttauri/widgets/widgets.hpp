// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ButtonWidget.hpp"
#include "LineInputWidget.hpp"
#include "ToggleWidget.hpp"
#include "CheckboxWidget.hpp"
#include "SelectionWidget.hpp"
#include "RadioButtonWidget.hpp"
#include "ToolbarWidget.hpp"
#include "WindowWidget.hpp"
#include "RowWidget.hpp"
#include "ColumnWidget.hpp"
#include "../GUI/Window.hpp"

namespace tt {

/** Add a widget to the main widget of the window.
* The implementation is located here so that widget is a concrete type.
*/
template<typename T, typename... Args>
T &Window_base::makeWidget(Args &&... args)
{
    tt_assume(widget);
    return widget->makeWidget<T>(std::forward<Args>(args)...);
}

}
