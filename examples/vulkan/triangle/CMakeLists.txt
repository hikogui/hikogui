# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_executable(hikogui_vulkan_triangle WIN32 MACOSX_BUNDLE)

target_link_libraries(hikogui_vulkan_triangle PRIVATE hikogui)
target_sources(hikogui_vulkan_triangle PRIVATE main.cpp)

add_custom_target(hikogui_vulkan_triangle_shaders)
add_shader_target(hikogui_vulkan_triangle_shaders
    shaders/triangle.vert
    shaders/triangle.frag
)

add_custom_target(hikogui_vulkan_triangle_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/../../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    hikogui hikogui_vulkan_triangle_shaders
    resources/vulkan_triangle.png
)

add_dependencies(hikogui_vulkan_triangle hikogui_vulkan_triangle_resources)

#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------

install(TARGETS hikogui_vulkan_triangle DESTINATION examples/vulkan/triangle)

install(DIRECTORY resources/ DESTINATION examples/vulkan/triangle/resources)

# copy additional "hikogui library" resources from top-level
install(DIRECTORY ../../resources/  DESTINATION examples/vulkan/triangle/resources)

# copy vcpkg dependencies (experimental)
if (VCPKG_TOOLCHAIN)
    x_vcpkg_install_local_dependencies(TARGETS vulkan_triangle DESTINATION examples/vulkan/triangle)
endif()

