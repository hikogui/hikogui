
cmake_minimum_required(VERSION 3.0)

project(TTauri CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(TTauriConfigParser Config/TTauriConfig.y ${CMAKE_CURRENT_SOURCE_DIR}/Config/TTauriConfigParser.cpp DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Config/TTauriConfigParserTokens.h)
FLEX_TARGET(TTauriConfigScanner Config/TTauriConfig.l ${CMAKE_CURRENT_SOURCE_DIR}/Config/TTauriConfigScanner.cpp DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Config/TTauriConfigParserTokens.h)
ADD_FLEX_BISON_DEPENDENCY(TTauriConfigScanner TTauriConfigParser)


set(TTauri_srcs
    ${BISON_TTauriConfigParser_OUTPUTS}
    ${BISON_TTauriConfigScanner_OUTPUTS}
    Config/Token.cpp
    Config/Parser.cpp
    Draw/PixelMap.cpp
    Draw/PNG.cpp
	Draw/FreeType.cpp
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

set(TTauri_GUI_shaders
	GUI/PipelineImage.vert
	GUI/PipelineImage.frag
)

file(GLOB_RECURSE TTauri_hdrs *.hpp)
source_group("Headers" FILES ${TTauri_hdrs})

add_library(TTauri STATIC ${TTauri_srcs})
target_link_libraries(TTauri GSL zlib png glm ${Boost_LIBRARIES} ${Vulkan_LIBRARIES} VMA freetype)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(TTauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_property(TARGET TTauri PROPERTY CXX_STANDARD 17)



foreach(shader ${TTauri_GUI_shaders})
	target_add_shader(TTauri ${shader})
endforeach(shader)
