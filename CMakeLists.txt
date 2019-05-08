
cmake_minimum_required(VERSION 3.0)

include(CMakeIncludes/AddShader.cmake)

if (MSVC)
    add_compile_options(/wd4068)
	add_definitions(-DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif ()


add_subdirectory(Utilities)
add_subdirectory(ThirdPartyLibraries)
add_subdirectory(TTauri)
add_subdirectory(TTauriDevelopmentApplication)
 
enable_testing()

add_test(NAME TTauriTests COMMAND TTauri/TTauriTests)
 
 

