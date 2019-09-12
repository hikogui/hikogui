
project(TTauri_GUI CXX)

find_package(Vulkan REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/TTauri/CMake")
include(AddShader)
include(AddStaticResource)

add_shader(TTauri_GUI_Shader_objs
    shaders/PipelineImage.vert
    shaders/PipelineImage.frag
    shaders/PipelineFlat.vert
    shaders/PipelineFlat.frag
)

set(TTauri_GUI_srcs
    src/init.cpp
    src/Device_base.cpp
    src/Device_vulkan.cpp
    src/Instance_base.cpp
    src/Instance_vulkan.cpp
    src/Instance_vulkan_win32.cpp
    src/Pipeline_base.cpp
    src/Pipeline_vulkan.cpp
    src/PipelineFlat.cpp
    src/PipelineFlat_DeviceShared.cpp
    src/PipelineImage.cpp
    src/PipelineImage_DeviceShared.cpp
    src/PipelineImage_Image.cpp
    src/PipelineImage_TextureMap.cpp
    src/VerticalSync_win32.cpp
    src/VulkanMemoryAllocator.cpp
    src/Window_base.cpp
    src/Window_vulkan.cpp
    src/Window_vulkan_win32.cpp
)

add_library(TTauri_GUI STATIC ${TTauri_GUI_srcs})
target_static_resource(TTauri_GUI ${TTauri_GUI_Shader_objs})
target_link_libraries(TTauri_GUI TTauri_Foundation TTauri_Draw ${Vulkan_LIBRARIES} VMA rhea)
target_include_directories(TTauri_GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${Vulkan_INCLUDE_DIRS})

if (MSVC)
    target_compile_definitions(TTauri_GUI PUBLIC -D_WIN64  -DVK_USE_PLATFORM_WIN32_KHR)
endif()
