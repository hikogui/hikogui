
cmake_minimum_required(VERSION 3.0)

project(TTauri CXX)

set(TTauri_Config_srcs
    Config/Token.cpp
)

set(TTauri_Draw_srcs
    Draw/PixelMap.cpp
    Draw/PNG.cpp
	Draw/FreeType.cpp
)

set(TTauri_GUI_srcs
    GUI/Device.cpp
    GUI/Device_vulkan.cpp
    GUI/ImageView.cpp
    GUI/Instance.cpp
    GUI/Instance_vulkan.cpp
    GUI/Instance_vulkan_win32.cpp
    GUI/Pipeline.cpp
    GUI/Pipeline_vulkan.cpp
    GUI/PipelineImage.cpp
    GUI/PipelineImage_DeviceShared.cpp
    GUI/PipelineImage_Image.cpp
    GUI/PipelineImage_TextureMap.cpp
    GUI/View.cpp
    GUI/vulkan_utils.cpp
    GUI/VulkanMemoryAllocator.cpp
    GUI/Window.cpp
    GUI/Window_vulkan.cpp
    GUI/Window_vulkan_win32.cpp
    GUI/WindowView.cpp
)

set(TTauri_GUI_shaders
	GUI/PipelineImage.vert
	GUI/PipelineImage.frag
)

set(TTauri_srcs
    ${TTauri_Config_srcs}
    ${TTauri_Draw_srcs}
    ${TTauri_GUI_srcs}
    Application.cpp
    Application_win32.cpp
    Logging.cpp
    strings.cpp
    Time.cpp
    Timestamp.cpp
)

set(test_srcs
    strings_tests.cpp
)

add_library(TTauri STATIC ${TTauri_srcs})
target_link_libraries(TTauri GSL zlib png glm ${Boost_LIBRARIES} ${Vulkan_LIBRARIES} VMA freetype)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(TTauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_property(TARGET TTauri PROPERTY CXX_STANDARD 17)

foreach(shader ${TTauri_GUI_shaders})
	target_add_shader(TTauri ${shader})
endforeach(shader)
