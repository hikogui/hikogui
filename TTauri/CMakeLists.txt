# Copyright 2019 Pokitec
# All rights reserved.

cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0074 NEW)


project(TTauri CXX)

add_subdirectory(Required)
add_subdirectory(Time)
add_subdirectory(Diagnostic)
add_subdirectory(Foundation)
add_subdirectory(Config)
add_subdirectory(Draw)
add_subdirectory(GUI)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/TTauri/CMake")
include(AddIcon)
include(AddStaticResource)

# Visual Studio likes to know about all header files.
#file(GLOB_RECURSE TTauri_hdrs *.hpp *.inl)
#source_group("Headers" FILES ${TTauri_hdrs})

add_icon(Icon_objs 
    "Themes/Icons/Application Icon.svg"
    "Themes/Icons/Close Window.svg"
    "Themes/Icons/Maximize Window.svg"
    "Themes/Icons/Minimize Window.svg"
    "Themes/Icons/MultiColor.svg"
)

set(TTauri_resources
    ${Icon_objs}
    Themes/Fonts/Roboto/Roboto-Regular.ttf
)

set(TTauri_srcs
    src/Application_base.cpp
    src/Application_win32.cpp
    src/StaticResources.cpp
)

#set(TTauri_do_not_analyze
#    ${BISON_TTauriConfigParser_OUTPUTS}
#    ${FLEX_TTauriConfigScanner_OUTPUTS}
#    ${TTauriTests_srcs}
#)

#if(MSVC)
#    add_compile_options(/wd4996 /wd4005)
#    if(TTAURI_ENABLE_CODE_ANALYSIS)
#        set_source_files_properties(${TTauri_do_not_analyze} PROPERTIES COMPILE_FLAGS /analyze-)
#        add_compile_options(/analyze /analyze:plugin EspXEngine.dll)
#        add_compile_options(/wd26447)
#       add_compile_options(/wd26486) # lifetime.3 tooooooo many false positives.
#   endif()
#endif()

add_library(TTauri STATIC ${TTauri_srcs})
target_static_resource(TTauri ${TTauri_resources})

set_property(TARGET TTauri PROPERTY CXX_STANDARD 17)
target_include_directories(TTauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(TTauri TTauri_Foundation TTauri_Config TTauri_GUI )
target_compile_definitions(TTauri PUBLIC -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL)

if (MSVC)
    target_compile_definitions(TTauri PUBLIC -D_WIN64 -DVK_USE_PLATFORM_WIN32_KHR)
endif()

#add_executable(TTauriTests ${TTauriTests_srcs})
#target_link_libraries(TTauriTests TTauri gtest_main)
#set_property(TARGET TTauriTests PROPERTY CXX_STANDARD 17)

#set(TestFileGlobs
#    ${CMAKE_CURRENT_SOURCE_DIR}/TestFiles/*
#    ${CMAKE_CURRENT_SOURCE_DIR}/Config/TestFiles/*
#    ${CMAKE_CURRENT_SOURCE_DIR}/Draw/TestFiles/*
#)

#file(GLOB TestFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS ${TestFileGlobs})
# foreach(TestFile ${TestFiles})
#     add_custom_command(TARGET TTauriTests POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy
#                    ${CMAKE_CURRENT_SOURCE_DIR}/${TestFile}
#                    ${CMAKE_CURRENT_BINARY_DIR}/${TestFile})
#endforeach()

