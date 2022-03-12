# Copyright Take Vos 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(ttauri PRIVATE
    gui_system.cpp
    gui_system.hpp
    gui_system_delegate.hpp
    gui_system_win32.cpp
    gui_system_win32.hpp
    gui_window.cpp
    gui_window.hpp
    gui_window_delegate.hpp
    gui_window_size.hpp
    $<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/gui_window_vulkan_macos.hpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/gui_window_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/gui_window_win32.hpp>
    hitbox.hpp
    keyboard_bindings.cpp
    keyboard_bindings.hpp
    keyboard_event.hpp
    keyboard_key.hpp
    keyboard_modifiers.hpp
    keyboard_virtual_key.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/keyboard_virtual_key_win32.cpp>
    keyboard_focus_direction.hpp
    keyboard_focus_group.hpp
    mouse_buttons.hpp
    mouse_cursor.hpp
    mouse_event.hpp
    theme.cpp
    theme.hpp
    theme_book.cpp
    theme_book.hpp
    theme_mode.hpp
    vertical_sync.cpp
    vertical_sync.hpp
    #$<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/vertical_sync_macos.hpp>
    #$<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/vertical_sync_macos.mm>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/vertical_sync_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/vertical_sync_win32.hpp>
)

if(TT_BUILD_PCH AND NOT TT_ENABLE_ANALYSIS)
    target_precompile_headers(ttauri PRIVATE
        gui_system.hpp
        gui_system_win32.hpp
    )
endif()