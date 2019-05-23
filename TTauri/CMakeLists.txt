
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0074 NEW)

project(TTauri CXX)

find_package(Boost REQUIRED)
find_package(Vulkan REQUIRED)


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
    Config/parser.cpp
    Draw/PixelMap.cpp
    Draw/PNG.cpp
    Draw/Font.cpp
    Draw/TrueTypeParser.cpp
    Draw/Theme.cpp
    GUI/globals.cpp
    GUI/VerticalSync.cpp
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
    GUI/VulkanMemoryAllocator.cpp
    GUI/Window.cpp
    GUI/Window_vulkan.cpp
    GUI/Window_vulkan_win32.cpp
    GUI/WindowView.cpp
    Application.cpp
    Application_win32.cpp
    logging.cpp
    File.cpp
    FileMapping.cpp
    FileView.cpp
    strings.cpp
    Time.cpp
    Timestamp.cpp
)


if(MSVC)
    add_compile_options(/wd4996 /wd4005)
endif()


add_library(TTauri STATIC ${TTauri_srcs})
target_link_libraries(TTauri GSL zlib png glm ${Boost_LIBRARIES} ${Vulkan_LIBRARIES} VMA utf8proc)
target_include_directories(TTauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_property(TARGET TTauri PROPERTY CXX_STANDARD 17)

target_include_directories(TTauri PUBLIC ${Boost_INCLUDE_DIRS})
target_link_directories(TTauri PUBLIC ${Boost_LIBRARY_DIRS})

target_include_directories(TTauri PUBLIC ${Vulkan_INCLUDE_DIRS})
if (MSVC)
    target_compile_definitions(TTauri PUBLIC -D_WIN64)
    target_compile_definitions(TTauri PUBLIC -DVK_USE_PLATFORM_WIN32_KHR)
endif()

target_compile_definitions(TTauri PUBLIC -DGLM_FORCE_SWIZZLE -DGLM_ENABLE_EXPERIMENTAL)


set(TTauri_GUI_shaders
	GUI/PipelineImage.vert
	GUI/PipelineImage.frag
)

foreach(shader ${TTauri_GUI_shaders})
	target_add_shader(TTauri ${shader})
endforeach(shader)

set(TTauriTests_srcs
    Config/parser_tests.cpp
    Config/Config_tests.cpp
    Draw/QBezier_tests.cpp
    Draw/TrueTypeParser_tests.cpp
    Draw/Path_tests.cpp
    Draw/PixelMap_tests.cpp
    FileView_tests.cpp
    strings_tests.cpp
    math_tests.cpp
)


add_executable(TTauriTests ${TTauriTests_srcs})
target_link_libraries(TTauriTests TTauri gtest_main)
set_property(TARGET TTauriTests PROPERTY CXX_STANDARD 17)

set(TestFileGlobs
    ${CMAKE_CURRENT_SOURCE_DIR}/TestFiles/*
    ${CMAKE_CURRENT_SOURCE_DIR}/Config/TestFiles/*
    ${CMAKE_CURRENT_SOURCE_DIR}/Draw/TestFiles/*
)

file(GLOB TestFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS ${TestFileGlobs})
 foreach(TestFile ${TestFiles})
     add_custom_command(TARGET TTauriTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_CURRENT_SOURCE_DIR}/${TestFile}
                    ${CMAKE_CURRENT_BINARY_DIR}/${TestFile})
endforeach()
