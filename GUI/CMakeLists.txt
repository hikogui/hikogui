
project(TTauri_GUI LANGUAGES)

find_package(Vulkan REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")
include(AddShader)
include(AddStaticResource)

add_shader(TTauri_GUI_Shader_objs
    shaders/PipelineImage.vert
    shaders/PipelineImage.frag
    shaders/PipelineFlat.vert
    shaders/PipelineFlat.frag
    shaders/PipelineBox.vert
    shaders/PipelineBox.frag
    shaders/PipelineSDF.vert
    shaders/PipelineSDF.frag
    shaders/PipelineToneMapper.vert
    shaders/PipelineToneMapper.frag
)

set(TTauri_GUI_srcs
    src/globals.cpp
    src/RenderDoc.cpp
    src/GUIDevice_base.cpp
    src/GUIDevice_vulkan.cpp
    src/GUISystem_base.cpp
    src/GUISystem_vulkan.cpp
    src/Pipeline_base.cpp
    src/Pipeline_vulkan.cpp
    src/PipelineFlat.cpp
    src/PipelineFlat_DeviceShared.cpp
    src/PipelineToneMapper.cpp
    src/PipelineToneMapper_DeviceShared.cpp
    src/PipelineBox.cpp
    src/PipelineBox_DeviceShared.cpp
    src/PipelineImage.cpp
    src/PipelineImage_DeviceShared.cpp
    src/PipelineImage_Image.cpp
    src/PipelineImage_TextureMap.cpp
    src/PipelineSDF.cpp
    src/PipelineSDF_DeviceShared.cpp
    src/PipelineSDF_TextureMap.cpp
    src/PipelineSDF_AtlasRect.cpp
    src/VulkanMemoryAllocator.cpp
    src/Window_base.cpp
    src/Window_vulkan.cpp
    src/Theme.cpp
    src/ThemeBook.cpp
    src/KeyboardBindings.cpp
    src/KeyboardEvent.cpp
    #src/Image.cpp
)

if(WIN32)
set(TTauri_GUI_srcs
    ${TTauri_GUI_srcs}
    src/GUISystem_vulkan_win32.cpp
    src/ThemeMode_win32.cpp
    src/Window_vulkan_win32.cpp
    src/VerticalSync_win32.cpp
    src/KeyboardVirtualKey_win32.cpp
)
elseif(APPLE)
set(TTauri_GUI_srcs
    ${TTauri_GUI_srcs}
    #src/GUISystem_vulkan_macos.mm
    #src/ThemeMode_macos.cpp
    #src/Window_vulkan_macos.cpp
    #src/VerticalSync_macos.cpp
)
endif()

add_library(TTauri_GUI STATIC ${TTauri_GUI_srcs})
target_static_resource(TTauri_GUI ${TTauri_GUI_Shader_objs})
target_link_libraries(TTauri_GUI TTauri_Foundation TTauri_Text ${Vulkan_LIBRARIES} VMA rhea RenderDoc)
target_include_directories(TTauri_GUI PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(TTauri_GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${Vulkan_INCLUDE_DIRS})

if (WIN32)
    target_compile_definitions(TTauri_GUI PUBLIC -D_WIN64 -DVK_USE_PLATFORM_WIN32_KHR)
    target_link_libraries(TTauri_GUI UxTheme.lib)
elseif(APPLE)
    target_compile_definitions(TTauri_GUI PUBLIC -DVK_USE_PLATFORM_METAL_EXT)
endif()
