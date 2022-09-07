# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

project(hikogui_utilities CXX)

#-------------------------------------------------------------------
# Build Target: embed_static_resource                   (executable)
#-------------------------------------------------------------------

add_executable(embed_static_resource embed_static_resource.cpp)
target_compile_features(embed_static_resource PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(embed_static_resource PRIVATE -wd4068)

    # Set defines to compile a win32 application.
	target_compile_options(embed_static_resource PRIVATE -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

# disable debug info (do not create PDB)
target_compile_options(embed_static_resource PRIVATE
    "$<$<CONFIG:Release,RelWithDebInfo>:-DEBUG:NONE>")
target_link_options(embed_static_resource PRIVATE
    "$<$<CONFIG:Release,RelWithDebInfo>:-DEBUG:NONE>")
