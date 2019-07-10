

function(target_static_resource TARGET)
    foreach(SOURCE_FILE IN LISTS ARGN)
        get_filename_component(INPUT_PATH ${SOURCE_FILE} ABSOLUTE)
        get_filename_component(INPUT_FILENAME ${SOURCE_FILE} NAME)
        get_relative_path(INPUT_RELPATH ${INPUT_PATH})

        get_filename_component(OUTPUT_RELDIR ${INPUT_RELPATH} DIRECTORY)
        set(OUTPUT_FILENAME "${INPUT_FILENAME}.inl")
        set(OUTPUT_RELPATH "${OUTPUT_RELDIR}/${OUTPUT_FILENAME}")
        get_filename_component(OUTPUT_PATH ${OUTPUT_RELPATH} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

	    add_custom_command(
		    OUTPUT ${OUTPUT_PATH}
		    COMMAND BinaryToCPP ${INPUT_PATH} ${OUTPUT_PATH}
		    DEPENDS ${INPUT_PATH} BinaryToCPP
		    VERBATIM
	    )
        
        string(MAKE_C_IDENTIFIER "${OUTPUT_FILENAME}.target" INTERMEDIATE_TARGET)
        add_custom_target(${INTERMEDIATE_TARGET} DEPENDS ${OUTPUT_PATH})

        add_dependencies(${TARGET} ${INTERMEDIATE_TARGET})
    endforeach()
endfunction()
