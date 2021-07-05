
#-------------------------------------------------------------------
# Build Target: ttauri_demo                             (executable)
#-------------------------------------------------------------------

add_custom_target(widget_example_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    ttauri
)

add_executable(checkbox_example WIN32 MACOSX_BUNDLE)
target_sources(checkbox_example PRIVATE checkbox_example.cpp)
target_link_libraries(checkbox_example PRIVATE ttauri)
target_include_directories(checkbox_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(checkbox_example widget_example_resources)

add_executable(radio_button_example WIN32 MACOSX_BUNDLE)
target_sources(radio_button_example PRIVATE radio_button_example.cpp)
target_link_libraries(radio_button_example PRIVATE ttauri)
target_include_directories(radio_button_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(radio_button_example widget_example_resources)


#-------------------------------------------------------------------
# Installation Rules: ttauri_demo
#-------------------------------------------------------------------

install(TARGETS checkbox_example DESTINATION examples/widgets)
install(TARGETS radio_button_example DESTINATION examples/widgets)

# copy additional "ttauri library" resources from top-level
install(DIRECTORY ../../resources/  DESTINATION examples/widgets/resources)

# copy vcpkg dependencies (experimental)
if (VCPKG_TOOLCHAIN)
    x_vcpkg_install_local_dependencies(TARGETS ttauri_demo DESTINATION examples/widgets)
endif()

