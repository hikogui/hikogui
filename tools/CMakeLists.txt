
project(ttauri_utilities C)

#-------------------------------------------------------------------
# Build Target: BinaryToCPP                             (executable)
#-------------------------------------------------------------------

add_executable(BinaryToCPP BinaryToCPP.c)

if (MSVC)
    target_compile_options(BinaryToCPP PUBLIC -wd4068)

    # Set defines to compile a win32 application.
	target_compile_options(BinaryToCPP PUBLIC -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)

    # std::result_of used by fmt.
    target_compile_options(BinaryToCPP PUBLIC -D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)
endif()
