# Copyright Take Vos 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(ttauri PRIVATE
    draw_context.cpp
    draw_context.hpp
    gfx_device.cpp
    gfx_device.hpp
    gfx_device_vulkan.cpp
    gfx_device_vulkan.hpp
    gfx_surface.cpp
    gfx_surface.hpp
    gfx_surface_state.hpp
    gfx_surface_vulkan.cpp
    gfx_surface_vulkan.hpp
    gfx_system.cpp
    gfx_system.hpp
    gfx_system_globals.hpp
    gfx_system_vulkan.cpp
    gfx_system_vulkan.hpp
    #$<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/gfx_system_vulkan_macos.hpp>
    #$<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/fgx_system_vulkan_macos.mm>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/gfx_system_vulkan_win32.cpp>
    paged_image.cpp
    paged_image.hpp
    pipeline.cpp
    pipeline.hpp
    pipeline_vulkan.cpp
    pipeline_vulkan.hpp
    pipeline_box.cpp
    pipeline_box.frag
    pipeline_box.hpp
    pipeline_box.vert
    pipeline_box_device_shared.cpp
    pipeline_box_device_shared.hpp
    pipeline_box_push_constants.hpp
    pipeline_box_vertex.hpp
    pipeline_image.cpp
    pipeline_image.frag
    pipeline_image.hpp
    pipeline_image.vert
    pipeline_image_device_shared.cpp
    pipeline_image_device_shared.hpp
    pipeline_image_push_constants.hpp
    pipeline_image_texture_map.cpp
    pipeline_image_texture_map.hpp
    pipeline_image_vertex.hpp
    pipeline_SDF.cpp
    pipeline_SDF.frag
    pipeline_SDF.hpp
    pipeline_SDF.vert
    pipeline_SDF_device_shared.cpp
    pipeline_SDF_device_shared.hpp
    pipeline_SDF_push_constants.hpp
    pipeline_SDF_specialization_constants.hpp
    pipeline_SDF_texture_map.cpp
    pipeline_SDF_texture_map.hpp
    pipeline_SDF_vertex.hpp
    pipeline_tone_mapper.cpp
    pipeline_tone_mapper.frag
    pipeline_tone_mapper.hpp
    pipeline_tone_mapper.vert
    pipeline_tone_mapper_device_shared.cpp
    pipeline_tone_mapper_device_shared.hpp
    pipeline_tone_mapper_push_constants.hpp
    RenderDoc.cpp
    RenderDoc.hpp
    subpixel_orientation.hpp
    VulkanMemoryAllocator.cpp
)

if(TT_BUILD_PCH AND NOT TT_ENABLE_ANALYSIS)
    target_precompile_headers(ttauri PRIVATE
        gfx_system.hpp
        gfx_system_vulkan.hpp
    )
endif()