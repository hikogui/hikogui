// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/widgets/ButtonWidget.hpp"
#include "ttauri/widgets/LineInputWidget.hpp"
#include "ttauri/widgets/ToggleWidget.hpp"
#include "ttauri/widgets/CheckboxWidget.hpp"
#include "ttauri/widgets/RadioButtonWidget.hpp"
#include "ttauri/widgets/ToolbarWidget.hpp"
#include "ttauri/widgets/WindowWidget.hpp"
#include "ttauri/widgets/RowWidget.hpp"
#include "ttauri/widgets/ColumnWidget.hpp"
#include "ttauri/GUI/Window.hpp"

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
