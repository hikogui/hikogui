# Copyright Take Vos 2020-2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    gui_event.hpp
    gui_event_type.hpp
    gui_event_variant.hpp
    gui_system.cpp
    gui_system.hpp
    gui_system_delegate.hpp
    gui_system_win32.cpp
    gui_system_win32.hpp
    gui_window.cpp
    gui_window.hpp
    gui_window_size.hpp
    $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/gui_window_vulkan_macos.hpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/gui_window_win32.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/gui_window_win32.hpp>
    hitbox.hpp
    keyboard_bindings.cpp
    keyboard_bindings.hpp
    keyboard_key.hpp
    keyboard_modifiers.hpp
    keyboard_virtual_key.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/keyboard_virtual_key_win32.cpp>
    keyboard_focus_direction.hpp
    keyboard_focus_group.hpp
    mouse_buttons.hpp
    mouse_cursor.hpp
    theme.cpp
    theme.hpp
    theme_book.cpp
    theme_book.hpp
    theme_mode.hpp
)

