
project(ttauri_utilities C)

#-------------------------------------------------------------------
# Build Target: embed_static_resource                   (executable)
#-------------------------------------------------------------------

add_executable(embed_static_resource embed_static_resource.cpp)

if (MSVC)
    target_compile_options(embed_static_resource PRIVATE -wd4068)

    # Set defines to compile a win32 application.
	target_compile_options(embed_static_resource PRIVATE -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()
