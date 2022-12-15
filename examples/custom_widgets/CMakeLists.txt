# Copyright Take Vos 2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_custom_target(custom_widget_example_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    hikogui
    resources/copyright.md
    resources/mars3.png
)

add_executable(minimum_custom_widget_example WIN32 MACOSX_BUNDLE)
target_sources(minimum_custom_widget_example PRIVATE minimum_custom_widget_example_impl.cpp)
target_link_libraries(minimum_custom_widget_example PRIVATE hikogui)
target_include_directories(minimum_custom_widget_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(minimum_custom_widget_example custom_widget_example_resources)

add_executable(custom_widget_with_child_example WIN32 MACOSX_BUNDLE)
target_sources(custom_widget_with_child_example PRIVATE custom_widget_with_child_example_impl.cpp)
target_link_libraries(custom_widget_with_child_example PRIVATE hikogui)
target_include_directories(custom_widget_with_child_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(custom_widget_with_child_example custom_widget_example_resources)

add_executable(custom_widget_command_example WIN32 MACOSX_BUNDLE)
target_sources(custom_widget_command_example PRIVATE custom_widget_command_example_impl.cpp)
target_link_libraries(custom_widget_command_example PRIVATE hikogui)
target_include_directories(custom_widget_command_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(custom_widget_command_example custom_widget_example_resources)

add_executable(custom_widget_drawing_example WIN32 MACOSX_BUNDLE)
target_sources(custom_widget_drawing_example PRIVATE custom_widget_drawing_example_impl.cpp)
target_link_libraries(custom_widget_drawing_example PRIVATE hikogui)
target_include_directories(custom_widget_drawing_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(custom_widget_drawing_example custom_widget_example_resources)

add_dependencies(examples
    minimum_custom_widget_example
    custom_widget_with_child_example
    custom_widget_command_example
    custom_widget_drawing_example)

#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------

install(TARGETS minimum_custom_widget_example DESTINATION examples/custom_widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(TARGETS custom_widget_with_child_example DESTINATION examples/custom_widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(TARGETS custom_widget_command_example DESTINATION examples/custom_widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(TARGETS custom_widget_drawing_example DESTINATION examples/custom_widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources DESTINATION examples/custom_widgets COMPONENT examples EXCLUDE_FROM_ALL)
