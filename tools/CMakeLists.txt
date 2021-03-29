
project(ttauri_utilities C)

#-------------------------------------------------------------------
# Build Target: embed_static_resource                   (executable)
#-------------------------------------------------------------------

add_executable(embed_static_resource embed_static_resource.cpp)
target_link_libraries(embed_static_resource PUBLIC fmt::fmt)

if (MSVC)
    target_compile_options(embed_static_resource PRIVATE -wd4068)

    # Set defines to compile a win32 application.
	target_compile_options(embed_static_resource PRIVATE -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)

    # std::result_of used by fmt.
    target_compile_options(embed_static_resource PRIVATE -D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)
endif()
