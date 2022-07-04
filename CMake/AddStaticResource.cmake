

function(target_static_resource TARGET)
    foreach(SOURCE_FILE IN LISTS ARGN)
        get_filename_component(INPUT_PATH "${SOURCE_FILE}" ABSOLUTE)
        get_filename_component(INPUT_FILENAME "${SOURCE_FILE}" NAME)
        get_relative_path(INPUT_RELPATH "${INPUT_PATH}")

        get_filename_component(OUTPUT_RELDIR "${INPUT_RELPATH}" DIRECTORY)
        set(OUTPUT_FILENAME "${INPUT_FILENAME}.cpp")
        set(OUTPUT_RELPATH "${OUTPUT_RELDIR}/${OUTPUT_FILENAME}")
        get_filename_component(OUTPUT_PATH "${OUTPUT_RELPATH}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

        # Create the output directory.
        get_filename_component(OUTPUT_DIR "${OUTPUT_PATH}" DIRECTORY)
        file(MAKE_DIRECTORY "${OUTPUT_DIR}")

	    add_custom_command(
		    OUTPUT "${OUTPUT_PATH}"
		    COMMAND embed_static_resource "${INPUT_PATH}" "${OUTPUT_PATH}"
		    DEPENDS "${INPUT_PATH}" embed_static_resource
		    VERBATIM
	    )
        
        target_sources(${TARGET} PRIVATE "${OUTPUT_PATH}")

        # Make sure the variable is included in the final executable so that it will be initialized.
        string(MAKE_C_IDENTIFIER "${INPUT_FILENAME}" SYMBOL_NAME)
        target_link_options(${TARGET} PUBLIC "/include:${SYMBOL_NAME}_srip")
    endforeach()
endfunction()
