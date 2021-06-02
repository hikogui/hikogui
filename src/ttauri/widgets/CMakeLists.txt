# Copyright Take Vos 2020-2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(ttauri PRIVATE
    abstract_button_widget.hpp
    audio_device_configuration_controller.cpp
    audio_device_configuration_controller.hpp
    button_delegate.hpp
    button_shape.hpp
    button_state.hpp
    button_type.hpp
    button_widget.hpp
    grid_layout_widget.cpp
    grid_layout_widget.hpp
    icon_widget.cpp
    icon_widget.cpp
    label_widget.cpp
    label_widget.hpp
    overlay_view_widget.hpp
    row_column_layout_widget.hpp
    scroll_bar_widget.hpp
    scroll_view_widget.hpp
    selection_widget.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/system_menu_widget.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/system_menu_widget.hpp>
    tab_view_widget.hpp
    text_field_delegate.hpp
    text_field_widget.hpp
    text_widget.cpp
    text_widget.hpp
    toolbar_tab_button_widget.hpp
    toolbar_widget.hpp
    widget.cpp
    widget.hpp
    widgets.hpp
    window_traffic_lights_widget.cpp
    window_traffic_lights_widget.hpp
    window_widget.cpp
    window_widget.hpp
)

if(TT_BUILD_PCH)
    if(NOT TT_ENABLE_ANALYSIS)
    target_precompile_headers(ttauri PRIVATE
        Widget.hpp
    )
    endif()
endif()
