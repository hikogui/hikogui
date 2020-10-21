
target_sources(ttauri PRIVATE
    abstract_bool_toggle_button_widget.hpp
    abstract_button_widget.hpp
    abstract_toggle_button_widget.hpp
    boolean_checkbox_widget.hpp
    ButtonWidget.hpp
    checkbox_widget.hpp
    ContainerWidget.cpp
    ContainerWidget.hpp
    GridLayoutWidget.cpp
    GridLayoutWidget.hpp
    GridLayoutDelegate.hpp
    LabelWidget.hpp
    LineInputWidget.cpp
    LineInputWidget.hpp
    overlay_view_widget.hpp
    RadioButtonWidget.hpp
    RowColumnLayoutWidget.hpp
    ScrollViewWidget.hpp
    ScrollBarWidget.hpp
    SelectionWidget.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.hpp>
    TabViewWidget.hpp
    toggle_widget.hpp
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
