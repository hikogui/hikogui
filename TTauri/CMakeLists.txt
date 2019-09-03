# Copyright 2019 Pokitec
# All rights reserved.

cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0074 NEW)

project(TTauri CXX)

include(CMakeIncludes/AddShader.cmake)
include(CMakeIncludes/AddIcon.cmake)
include(CMakeIncludes/AddStaticResource.cmake)

find_package(Boost REQUIRED)
find_package(Vulkan REQUIRED)

# Visual Studio likes to know about all header files.
file(GLOB_RECURSE TTauri_hdrs *.hpp *.inl)
source_group("Headers" FILES ${TTauri_hdrs})

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Config)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
BISON_TARGET(TTauriConfigParser Config/TTauriConfig.y ${CMAKE_CURRENT_BINARY_DIR}/Config/TTauriConfigParser.cpp COMPILE_FLAGS "-p TTauriConfig_yy" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Config/TTauriConfigParser.hpp)
FLEX_TARGET(TTauriConfigScanner Config/TTauriConfig.l ${CMAKE_CURRENT_BINARY_DIR}/Config/TTauriConfigScanner.cpp COMPILE_FLAGS "-P TTauriConfig_yy" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Config/TTauriConfigScanner.hpp)
ADD_FLEX_BISON_DEPENDENCY(TTauriConfigScanner TTauriConfigParser)

add_shader(Shader_objs
    GUI/PipelineImage.vert
    GUI/PipelineImage.frag
    GUI/PipelineFlat.vert
    GUI/PipelineFlat.frag
)

add_icon(Icon_objs 
    "Themes/Icons/Application Icon.svg"
    "Themes/Icons/Close Window.svg"
    "Themes/Icons/Maximize Window.svg"
    "Themes/Icons/Minimize Window.svg"
    "Themes/Icons/MultiColor.svg"
)

set(TTauri_resources
    ${Shader_objs}
    ${Icon_objs}
    Themes/Fonts/Roboto/Roboto-Regular.ttf
)

set(TTauri_srcs
    ${BISON_TTauriConfigParser_OUTPUTS}
    ${FLEX_TTauriConfigScanner_OUTPUTS}
    Config/LiteralParser.cpp
    Config/parser.cpp
    Draw/PNG.cpp
    Draw/PixelMap.cpp
    Draw/Path.cpp
    Draw/Font.cpp
    Draw/PathString.cpp
    Draw/BezierCurve.cpp
    Draw/TrueTypeParser.cpp
    Draw/TTauriIconParser.cpp
    Draw/Theme.cpp
    GUI/Device_base.cpp
    GUI/Device_vulkan.cpp
    GUI/Instance_base.cpp
    GUI/Instance_vulkan.cpp
    GUI/Instance_vulkan_win32.cpp
    GUI/VerticalSync_win32.cpp
    GUI/Pipeline_base.cpp
    GUI/Pipeline_vulkan.cpp
    GUI/PipelineFlat.cpp
    GUI/PipelineFlat_DeviceShared.cpp
    GUI/PipelineImage.cpp
    GUI/PipelineImage_DeviceShared.cpp
    GUI/PipelineImage_Image.cpp
    GUI/PipelineImage_TextureMap.cpp
    GUI/VulkanMemoryAllocator.cpp
    GUI/Window_base.cpp
    GUI/Window_vulkan.cpp
    GUI/Window_vulkan_win32.cpp
    GUI/Widgets/Widget.cpp
    GUI/Widgets/WindowWidget.cpp
    GUI/Widgets/WindowToolbarWidget.cpp
    GUI/Widgets/WindowTrafficLightsWidget.cpp
    GUI/Widgets/ImageWidget.cpp
    GUI/Widgets/ButtonWidget.cpp
    GUI/Widgets/ToolbarButtonWidget.cpp
    Application_base.cpp
    Application_win32.cpp
    datum.cpp
    hires_utc_clock.cpp
    hires_utc_clock_win32.cpp
    logger.cpp
    url_parser.cpp
    URL.cpp
    StaticResources.cpp
    StaticResourceView.cpp
    File.cpp
    FileMapping.cpp
    FileView.cpp
    strings.cpp
    wsRGBA.cpp
)

set(TTauriTests_srcs
    Config/parser_tests.cpp
    Config/Config_tests.cpp
    Draw/BezierCurve_tests.cpp
    Draw/TrueTypeParser_tests.cpp
    Draw/Path_tests.cpp
    Draw/PixelMap_tests.cpp
    datum_tests.cpp
    counters_tests.cpp
    string_tag_tests.cpp
    wsRGBA_tests.cpp
    FileView_tests.cpp
    strings_tests.cpp
    polymorphic_value_tests.cpp
    polynomial_tests.cpp
    URL_tests.cpp
    utils_tests.cpp
    pickle_tests.cpp
    small_map_tests.cpp
    exceptions_tests.cpp
)

set(TTauri_do_not_analyze
    ${BISON_TTauriConfigParser_OUTPUTS}
    ${FLEX_TTauriConfigScanner_OUTPUTS}
    ${TTauriTests_srcs}
)

if(MSVC)
    add_compile_options(/wd4996 /wd4005)
    if(TTAURI_ENABLE_CODE_ANALYSIS)
        set_source_files_properties(${TTauri_do_not_analyze} PROPERTIES COMPILE_FLAGS /analyze-)
        add_compile_options(/analyze /analyze:plugin EspXEngine.dll)
        add_compile_options(/wd26447)
    endif()
endif()

add_library(TTauri STATIC ${TTauri_srcs})
target_static_resource(TTauri ${TTauri_resources})

set_property(TARGET TTauri PROPERTY CXX_STANDARD 17)
target_include_directories(TTauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ${Boost_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
target_link_directories(TTauri PUBLIC ${Boost_LIBRARY_DIRS})
target_link_libraries(TTauri GSL zlib png glm fmt tz ${Boost_LIBRARIES} ${Vulkan_LIBRARIES} VMA utf8proc rhea)
target_compile_definitions(TTauri PUBLIC -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL)

if (MSVC)
    target_compile_definitions(TTauri PUBLIC -D_WIN64 -DVK_USE_PLATFORM_WIN32_KHR)
endif()

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

