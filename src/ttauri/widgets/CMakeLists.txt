
target_sources(ttauri PRIVATE
    ButtonWidget.cpp
    ButtonWidget.hpp
    CheckboxWidget.hpp
    ColumnWidget.cpp
    ColumnWidget.hpp
    ContainerWidget.hpp
    LabelWidget.hpp
    LineInputWidget.cpp
    LineInputWidget.hpp
    RadioButtonWidget.hpp
    RowWidget.cpp
    RowWidget.hpp
    SelectionWidget.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.hpp>
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
