
include(GetRelativePath)

function(add_shader RET)

    # add_shader depends on Vulkan::glslc. if not found, guide the user to find vulkan and the executable.
    if(NOT Vulkan_FOUND)
        message(FATAL_ERROR
            "The addShader() function depends on the \"glslc\" shader compiler executable.\n"
            "It is detected during find_package(Vulkan REQUIRED) and defined as imported target executable Vulkan::glslc.\n"
            "Please use find_package(Vulkan REQUIRED) and make sure it succeeds!\n"
        )
    endif()

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
