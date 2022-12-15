# Copyright Take Vos 2021-2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_custom_target(widget_example_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    hikogui
)

add_executable(checkbox_example WIN32 MACOSX_BUNDLE)
target_sources(checkbox_example PRIVATE checkbox_example_impl.cpp)
target_link_libraries(checkbox_example PRIVATE hikogui)
target_include_directories(checkbox_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(checkbox_example widget_example_resources)

add_executable(text_example WIN32 MACOSX_BUNDLE)
target_sources(text_example PRIVATE text_example_impl.cpp)
target_link_libraries(text_example PRIVATE hikogui)
target_include_directories(text_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(text_example widget_example_resources)

add_executable(toggle_example WIN32 MACOSX_BUNDLE)
target_sources(toggle_example PRIVATE toggle_example_impl.cpp)
target_link_libraries(toggle_example PRIVATE hikogui)
target_include_directories(toggle_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(toggle_example widget_example_resources)

add_executable(radio_button_example WIN32 MACOSX_BUNDLE)
target_sources(radio_button_example PRIVATE radio_button_example_impl.cpp)
target_link_libraries(radio_button_example PRIVATE hikogui)
target_include_directories(radio_button_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(radio_button_example widget_example_resources)

add_executable(tab_example WIN32 MACOSX_BUNDLE)
target_sources(tab_example PRIVATE tab_example_impl.cpp)
target_link_libraries(tab_example PRIVATE hikogui)
target_include_directories(tab_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(tab_example widget_example_resources)

add_executable(selection_example WIN32 MACOSX_BUNDLE)
target_sources(selection_example PRIVATE selection_example_impl.cpp)
target_link_libraries(selection_example PRIVATE hikogui)
target_include_directories(selection_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(selection_example widget_example_resources)

add_dependencies(examples
    checkbox_example
    text_example
    toggle_example
    radio_button_example
    tab_example
    selection_example)

#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------
install(TARGETS checkbox_example DESTINATION examples/widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(TARGETS text_example DESTINATION examples/widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(TARGETS toggle_example DESTINATION examples/widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(TARGETS radio_button_example DESTINATION examples/widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(TARGETS tab_example DESTINATION examples/widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(TARGETS selection_example DESTINATION examples/widgets COMPONENT examples EXCLUDE_FROM_ALL)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources  DESTINATION examples/widgets COMPONENT examples EXCLUDE_FROM_ALL)
