
cmake_minimum_required(VERSION 3.0)

project(TTauri CXX)



# Visual Studio likes to know about all header files.
file(GLOB_RECURSE TTauri_hdrs *.hpp)
source_group("Headers" FILES ${TTauri_hdrs})

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
BISON_TARGET(TTauriConfigParser Config/TTauriConfig.y ${CMAKE_CURRENT_BINARY_DIR}/Config/TTauriConfigParser.cpp COMPILE_FLAGS "-p TTauriConfig_yy" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Config/TTauriConfigParser.hpp)
FLEX_TARGET(TTauriConfigScanner Config/TTauriConfig.l ${CMAKE_CURRENT_BINARY_DIR}/Config/TTauriConfigScanner.cpp COMPILE_FLAGS "-P TTauriConfig_yy" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Config/TTauriConfigScanner.hpp)
ADD_FLEX_BISON_DEPENDENCY(TTauriConfigScanner TTauriConfigParser)

set(TTauri_srcs
    ${BISON_TTauriConfigParser_OUTPUTS}
    ${FLEX_TTauriConfigScanner_OUTPUTS}
    Config/LiteralParser.cpp
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


if(MSVC)
    add_compile_options(/wd4996 /wd4005)
endif()

add_library(TTauri STATIC ${TTauri_srcs})
target_link_libraries(TTauri GSL zlib png glm ${Boost_LIBRARIES} ${Vulkan_LIBRARIES} VMA freetype)
target_include_directories(TTauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_property(TARGET TTauri PROPERTY CXX_STANDARD 17)

set(TTauri_GUI_shaders
	GUI/PipelineImage.vert
	GUI/PipelineImage.frag
)

foreach(shader ${TTauri_GUI_shaders})
	target_add_shader(TTauri ${shader})
endforeach(shader)

set(TTauriTests_srcs
    Config/Parser_tests.cpp
)


add_executable(TTauriTests ${TTauriTests_srcs})
target_link_libraries(TTauriTests TTauri gtest_main)
set_property(TARGET TTauriTests PROPERTY CXX_STANDARD 17)

file(GLOB TestFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Config/TestFiles/*)
 foreach(TestFile ${TestFiles})
     add_custom_command(TARGET TTauriTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_CURRENT_SOURCE_DIR}/${TestFile}
                    ${CMAKE_CURRENT_BINARY_DIR}/${TestFile})
endforeach()
