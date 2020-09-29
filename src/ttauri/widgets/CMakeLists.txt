
target_sources(ttauri PRIVATE
    ButtonWidget.cpp
    ButtonWidget.hpp
    CheckboxWidget.hpp
    ColumnWidget.hpp
    ContainerWidget.cpp
    ContainerWidget.hpp
    GridWidget.cpp
    GridWidget.hpp
    GridWidgetDelegate.hpp
    LabelWidget.hpp
    LineInputWidget.cpp
    LineInputWidget.hpp
    RadioButtonWidget.hpp
    RowWidget.hpp
    SelectionWidget.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.hpp>
    TabWidget.hpp
    ToggleWidget.hpp
    ToolbarButtonWidget.hpp
    ToolbarTabButtonWidget.hpp
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
