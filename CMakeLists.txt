
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0054 NEW)
enable_testing()

project(TTauri LANGUAGES CXX)

#find_package(Doxygen)

set(BUILD_TTAURI_GUI OFF CACHE BOOL "Enable TTAURI GUI") 
set(BUILD_TTAURI_AUDIO OFF CACHE BOOL "Enable TTAURI Audio") 
set(BUILD_TTAURI_NET OFF CACHE BOOL "Enable TTAURI Networking") 

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")
include(AddIcon)
include(AddStaticResource)



add_subdirectory(ThirdPartyLibraries)
add_subdirectory(Utilities)

add_subdirectory(Foundation)
if(BUILD_TTAURI_NET)
message("TTauri Networking enabled")
add_subdirectory(Net)
endif()
if(BUILD_TTAURI_AUDIO)
message("TTauri Audio enabled")
add_subdirectory(Audio)
endif()
if(BUILD_TTAURI_GUI)
message("TTauri GUI enabled")
add_subdirectory(Text)
add_subdirectory(GUI)
add_subdirectory(Widgets)
endif()
add_subdirectory(Application)

add_library(TTauri INTERFACE)
target_link_libraries(TTauri INTERFACE TTauri_Application)

if(BUILD_TTAURI_GUI AND BUILD_TTAURI_AUDIO)
add_subdirectory(TTauriDevelopmentApplication)
endif()

#doxygen_add_docs(TTauri_Doxygen
#    Foundation/include/
#)

#set(TTauri_do_not_analyze
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



