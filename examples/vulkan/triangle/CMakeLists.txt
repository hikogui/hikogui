# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_executable(vulkan_triangle WIN32 MACOSX_BUNDLE)
add_dependencies(examples vulkan_triangle)

target_link_libraries(vulkan_triangle PRIVATE hikogui)
target_sources(vulkan_triangle PRIVATE main_impl.cpp triangle_impl.cpp triangle.hpp)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/triangle.frag.spv"
    COMMAND Vulkan::glslc
        -I "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/triangle.frag.spv"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.frag"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.frag"
    VERBATIM)

set(vulkan_triangle_compiled_shaders "${CMAKE_CURRENT_BINARY_DIR}/shaders/triangle.frag.spv")

add_custom_target(vulkan_triangle_shaders DEPENDS ${vulkan_triangle_compiled_shaders})
add_dependencies(vulkan_triangle vulkan_triangle_shaders)

set(vulkan_triangle_resources "${CMAKE_CURRENT_SOURCE_DIR}/resources/vulkan_triangle.png")

#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------
install(TARGETS vulkan_triangle DESTINATION examples/vulkan/triangle COMPONENT examples EXCLUDE_FROM_ALL)
install(FILES ${vulkan_triangle_compiled_shaders} DESTINATION examples/vulkan/triangle COMPONENT examples EXCLUDE_FROM_ALL)
install(FILES ${vulkan_triangle_resources} DESTINATION examples/vulkan/triangle COMPONENT examples EXCLUDE_FROM_ALL)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources DESTINATION examples/vulkan/triangle COMPONENT examples EXCLUDE_FROM_ALL)
