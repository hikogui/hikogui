# Copyright Take Vos 2021-2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    abstract_button_widget.cpp
    abstract_button_widget.hpp
    audio_device_widget.cpp
    audio_device_widget.hpp
    button_delegate.hpp
    checkbox_widget.cpp
    checkbox_widget.hpp
    grid_layout.hpp
    grid_layout.cpp
    grid_widget.cpp
    grid_widget.hpp
    icon_widget.cpp
    icon_widget.hpp
    label_widget.cpp
    label_widget.hpp
    menu_button_widget.cpp
    menu_button_widget.hpp
    momentary_button_widget.cpp
    momentary_button_widget.hpp
    overlay_widget.cpp
    overlay_widget.hpp
    radio_button_widget.cpp
    radio_button_widget.hpp
    row_column_widget.hpp
    scroll_bar_widget.hpp
    scroll_widget.hpp
    selection_delegate.hpp
    selection_widget.cpp
    selection_widget.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/system_menu_widget.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/system_menu_widget.hpp>
    tab_delegate.hpp
    tab_widget.cpp
    tab_widget.hpp
    text_delegate.hpp
    text_field_delegate.hpp
    text_field_widget.cpp
    text_field_widget.hpp
    text_widget.cpp
    text_widget.hpp
    toggle_widget.cpp
    toggle_widget.hpp
    toolbar_button_widget.cpp
    toolbar_button_widget.hpp
    toolbar_tab_button_widget.cpp
    toolbar_tab_button_widget.hpp
    toolbar_widget.cpp
    toolbar_widget.hpp
    widget.cpp
    widget.hpp
    widget_baseline.hpp
    widget_constraints.hpp
    widget_layout.hpp
    widget_mode.hpp
    window_traffic_lights_widget.cpp
    window_traffic_lights_widget.hpp
    window_widget.cpp
    window_widget.hpp
)

