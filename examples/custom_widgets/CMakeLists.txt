
#-------------------------------------------------------------------
# Build Target: ttauri_demo                             (executable)
#-------------------------------------------------------------------

add_custom_target(custom_widget_example_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    ttauri
)

add_executable(minimum_custom_widget_example WIN32 MACOSX_BUNDLE)
target_sources(minimum_custom_widget_example PRIVATE minimum_custom_widget_example.cpp)
target_link_libraries(minimum_custom_widget_example PRIVATE ttauri)
target_include_directories(minimum_custom_widget_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(minimum_custom_widget_example custom_widget_example_resources)

#-------------------------------------------------------------------
# Installation Rules: ttauri_demo
#-------------------------------------------------------------------

install(TARGETS minimum_custom_widget_example DESTINATION examples/custom_widgets)

# copy additional "ttauri library" resources from top-level
install(DIRECTORY ../../resources/  DESTINATION examples/custom_widgets/resources)

# copy vcpkg dependencies (experimental)
if (VCPKG_TOOLCHAIN)
    x_vcpkg_install_local_dependencies(TARGETS ttauri_demo DESTINATION examples/custom_widgets)
endif()

