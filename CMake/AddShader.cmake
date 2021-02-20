
include(GetRelativePath)

function(add_shader RET)

    # The "glslc" shader compiler executable is detected during find_package(VULKAN).
    # And defined as "Vulkan_GLSLC_EXECUTABLE" and also as target executable "Vulkan::glslc".

    foreach(SOURCE_FILE IN LISTS ARGN)
        message("add_shader: ${SOURCE_FILE}")
	    get_filename_component(INPUT_PATH ${SOURCE_FILE} ABSOLUTE)
        get_filename_component(INPUT_FILENAME ${SOURCE_FILE} NAME)
        get_relative_path(INPUT_RELPATH ${INPUT_PATH})

        get_filename_component(OUTPUT_RELDIR ${INPUT_RELPATH} DIRECTORY)
        set(OUTPUT_FILENAME "${INPUT_FILENAME}.spv")
        set(OUTPUT_RELPATH "${OUTPUT_RELDIR}/${OUTPUT_FILENAME}")
        get_filename_component(OUTPUT_PATH ${OUTPUT_RELPATH} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

        # Create the output directory.
        get_filename_component(OUTPUT_DIR ${OUTPUT_PATH} DIRECTORY)
        file(MAKE_DIRECTORY ${OUTPUT_DIR})

	    # Add a custom command to compile GLSL to SPIR-V.
	    add_custom_command(
		    OUTPUT ${OUTPUT_PATH}
		    COMMAND Vulkan::glslc -o ${OUTPUT_PATH} ${INPUT_PATH}
		    DEPENDS ${INPUT_PATH}
		    VERBATIM)

        set(OUTPUT_PATHS ${OUTPUT_PATHS} ${OUTPUT_PATH})
    endforeach()

    set(${RET} ${OUTPUT_PATHS} PARENT_SCOPE)
endfunction()
