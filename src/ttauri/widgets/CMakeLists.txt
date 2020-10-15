
target_sources(ttauri PRIVATE
    ButtonWidget.cpp
    ButtonWidget.hpp
    CheckboxWidget.hpp
    ContainerWidget.cpp
    ContainerWidget.hpp
    GridLayoutWidget.cpp
    GridLayoutWidget.hpp
    GridLayoutDelegate.hpp
    LabelWidget.hpp
    LineInputWidget.cpp
    LineInputWidget.hpp
    RadioButtonWidget.hpp
    RowColumnLayoutWidget.hpp
    ScrollViewWidget.hpp
    ScrollBarWidget.hpp
    SelectionWidget.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.hpp>
    TabViewWidget.hpp
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
