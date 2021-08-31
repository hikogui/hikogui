
project(ttauri_utilities C)

#-------------------------------------------------------------------
# Build Target: embed_static_resource                   (executable)
#-------------------------------------------------------------------

add_executable(embed_static_resource embed_static_resource.cpp)

if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
    target_compile_features(embed_static_resource PUBLIC cxx_std_20)
else()
    # CMake uses the incorrect /std:c++20 on MSVC when cxx_std_20 is used.
    target_compile_features(embed_static_resource PUBLIC cxx_std_23)
endif()

if (MSVC)
    target_compile_options(embed_static_resource PRIVATE -wd4068)

    # Set defines to compile a win32 application.
	target_compile_options(embed_static_resource PRIVATE -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()
