# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_executable(vulkan_triangle WIN32 MACOSX_BUNDLE)
add_dependencies(examples vulkan_triangle)

target_link_libraries(vulkan_triangle PRIVATE hikogui)
target_sources(vulkan_triangle PRIVATE main.cpp triangle.cpp triangle.hpp)

add_custom_target(vulkan_triangle_shaders)
add_shader_target(vulkan_triangle_shaders
    shaders/triangle.vert
    shaders/triangle.frag
)

add_custom_target(vulkan_triangle_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders
    DEPENDS
    hikogui vulkan_triangle_shaders
    resources/vulkan_triangle.png
)

add_dependencies(vulkan_triangle vulkan_triangle_resources)


#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------
install(TARGETS vulkan_triangle DESTINATION examples/vulkan/triangle COMPONENT examples EXCLUDE_FROM_ALL)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources DESTINATION examples/vulkan/triangle COMPONENT examples EXCLUDE_FROM_ALL)
