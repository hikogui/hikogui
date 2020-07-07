
target_sources(ttauri PRIVATE
    ButtonWidget.cpp
    ButtonWidget.hpp
    CheckboxWidget.hpp
    ColumnWidget.cpp
    ColumnWidget.hpp
    ContainerWidget.hpp
    globals.cpp
    globals.hpp
    LineInputWidget.cpp
    LineInputWidget.hpp
    RadioButtonWidget.hpp
    RowWidget.cpp
    RowWidget.hpp
    SystemMenuWidget.cpp
    SystemMenuWidget.hpp
    ToggleWidget.hpp
    ToolbarButtonWidget.cpp
    ToolbarButtonWidget.hpp
    ToolbarWidget.cpp
    ToolbarWidget.hpp
    Widget.cpp
    Widget.hpp
    widgets.hpp
    WindowTrafficLightsWidget.cpp
    WindowTrafficLightsWidget.hpp
    WindowWidget.cpp
    WindowWidget.hpp
)

target_precompile_headers(ttauri PRIVATE
    Widget.hpp
)
