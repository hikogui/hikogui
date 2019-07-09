

function(add_shader RET)
	# Find glslc shader compiler.
	# On Android, the NDK includes the binary, so no external dependency.
	if(ANDROID)
		file(GLOB glslc-folders ${ANDROID_NDK}/shader-tools/*)
		find_program(GLSLC glslc HINTS ${glslc-folders})
	else()
		find_program(GLSLC glslc)
	endif()

    foreach(SOURCE_FILE IN LISTS ARGN)
        message("add_shader: ${SOURCE_FILE}")
	    get_filename_component(INPUT_PATH ${SOURCE_FILE} ABSOLUTE)
        get_filename_component(INPUT_FILENAME ${SOURCE_FILE} NAME)

        set(OUTPUT_FILENAME "${INPUT_FILENAME}.spv")
        get_filename_component(OUTPUT_PATH ${OUTPUT_FILENAME} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

	    # Add a custom command to compile GLSL to SPIR-V.
	    add_custom_command(
		    OUTPUT ${OUTPUT_PATH}
		    COMMAND ${GLSLC} -o ${OUTPUT_PATH} ${INPUT_PATH}
		    DEPENDS ${INPUT_PATH}
		    VERBATIM)

        set(OUTPUT_PATHS ${OUTPUT_PATHS} ${OUTPUT_PATH})
    endforeach()

    set(${RET} ${OUTPUT_PATHS} PARENT_SCOPE)
endfunction()
