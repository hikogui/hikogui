# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_executable(hikogui_demo WIN32 MACOSX_BUNDLE)
add_dependencies(examples hikogui_demo)

target_link_libraries(hikogui_demo PRIVATE hikogui)
target_include_directories(hikogui_demo PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)

add_custom_target(hikogui_demo_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    hikogui
    resources/hikogui_demo.png
    resources/locale/en.po
    resources/locale/nl.po
)

add_dependencies(hikogui_demo hikogui_demo_resources)


#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------

install(TARGETS hikogui_demo DESTINATION examples/hikogui_demo COMPONENT examples EXCLUDE_FROM_ALL)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources  DESTINATION examples/hikogui_demo COMPONENT examples EXCLUDE_FROM_ALL)

