// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/ButtonWidget.hpp"
#include "TTauri/Widgets/LineInputWidget.hpp"
#include "TTauri/Widgets/ToggleWidget.hpp"
#include "TTauri/Widgets/CheckboxWidget.hpp"
#include "TTauri/Widgets/RadioButtonWidget.hpp"
#include "TTauri/Widgets/ToolbarWidget.hpp"
#include "TTauri/Widgets/WindowWidget.hpp"
#include "TTauri/GUI/Window.hpp"

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
