
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0074 NEW)

include(CMakeIncludes/AddShader.cmake)

if (MSVC)
    add_compile_options(/wd4068)
	add_definitions(-DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif ()

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)

add_subdirectory(Utilities)
add_subdirectory(ThirdPartyLibraries)
add_subdirectory(TTauri)
add_subdirectory(TTauriDevelopmentApplication)
#add_subdirectory(TTauriUnitTests)
 
