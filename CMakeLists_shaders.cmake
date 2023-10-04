
set(hikogui_shader_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/box_vulkan.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/box_vulkan.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/override_vulkan.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/override_vulkan.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/image_vulkan.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/image_vulkan.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/SDF_vulkan.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/SDF_vulkan.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/tone_mapper_vulkan.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/tone_mapper_vulkan.vert)

set(hikogui_shader_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(hikogui_compiled_shader_files "")
foreach(shader_file ${hikogui_shader_sources})
    set(input_file "${shader_file}")
    cmake_path(RELATIVE_PATH shader_file BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${shader_file}.spv")

    add_custom_command(
        OUTPUT "${output_file}"
        COMMAND Vulkan::glslc -I "${hikogui_shader_include_dir}" -o "${output_file}" "${input_file}"
        DEPENDS "${input_file}" "${hikogui_shader_include_dir}/utils_vulkan.glsl"
        VERBATIM)

    set(hikogui_compiled_shader_files ${hikogui_compiled_shader_files} "${output_file}")
endforeach()

add_custom_target(hikogui_shaders ALL DEPENDS ${hikogui_compiled_shader_files})
add_dependencies(hikogui hikogui_shaders)

target_resources(hikogui ${hikogui_compiled_shader_files})
