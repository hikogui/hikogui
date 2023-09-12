# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_executable(hikogui_demo WIN32 MACOSX_BUNDLE)
add_dependencies(examples hikogui_demo)

target_sources(hikogui_demo PRIVATE
    src/main_impl.cpp
)

target_link_libraries(hikogui_demo PRIVATE hikogui)

set(hikogui_demo_resource_files "")
set(hikogui_demo_resource_files ${hikogui_demo_resource_files} resources/en.po)
set(hikogui_demo_resource_files ${hikogui_demo_resource_files} resources/nl.po)
set(hikogui_demo_resource_files ${hikogui_demo_resource_files} resources/hikogui_demo.png)

set_target_properties(
    hikogui PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_FRAMEWORK_IDENTIFIER org.hikogui.hikogui.hikogui_demo
    RESOURCE "${hikogui_demo_resource_files}")

#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------

install(TARGETS hikogui_demo DESTINATION examples/hikogui_demo RESOURCE DESTINATION examples/hikogui_demo/resources COMPONENT examples EXCLUDE_FROM_ALL)

