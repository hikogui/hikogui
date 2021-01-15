
target_sources(ttauri PRIVATE
    abstract_bool_toggle_button_widget.hpp
    abstract_button_widget.hpp
    abstract_container_widget.hpp
    abstract_toggle_button_widget.hpp
    boolean_checkbox_widget.hpp
    button_widget.hpp
    checkbox_widget.hpp
    grid_layout_widget.cpp
    grid_layout_widget.hpp
    grid_layout_delegate.hpp
    LabelWidget.hpp
    text_field_delegate.hpp
    text_field_widget.hpp
    overlay_view_widget.hpp
    radio_button_widget.hpp
    row_column_layout_widget.hpp
    ScrollViewWidget.hpp
    ScrollBarWidget.hpp
    selection_widget.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/SystemMenuWidget.hpp>
    tab_view_widget.hpp
    toggle_widget.hpp
    menu_item_widget.hpp
    toolbar_tab_button_widget.hpp
    toolbar_widget.hpp
    widget.cpp
    widget.hpp
    widgets.hpp
    WindowTrafficLightsWidget.cpp
    WindowTrafficLightsWidget.hpp
    WindowWidget.cpp
    WindowWidget.hpp
)

if(NOT TTAURI_ENABLE_CODE_ANALYSIS)
target_precompile_headers(ttauri PRIVATE
    Widget.hpp
)
endif()
