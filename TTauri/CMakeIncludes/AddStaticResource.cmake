

function(target_static_resource TARGET)
    foreach(SOURCE_FILE IN LISTS ARGN)
        get_filename_component(INPUT_PATH ${SOURCE_FILE} ABSOLUTE)
        get_filename_component(INPUT_FILENAME ${SOURCE_FILE} NAME)

        set(OUTPUT_FILENAME ${INPUT_FILENAME}.inl)
        get_filename_component(OUTPUT_PATH ${OUTPUT_FILENAME} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})
        set(RESOURCE_URL "static-resource:${INPUT_FILENAME}")

	    add_custom_command(
		    OUTPUT ${OUTPUT_PATH}
		    COMMAND BinaryToCPP ${INPUT_PATH} ${OUTPUT_PATH} ${RESOURCE_URL}
		    DEPENDS ${INPUT_PATH} BinaryToCPP
		    VERBATIM
	    )
        
        string(MAKE_C_IDENTIFIER "${OUTPUT_FILENAME}.target" INTERMEDIATE_TARGET)
        add_custom_target(${INTERMEDIATE_TARGET} DEPENDS ${OUTPUT_PATH})

        add_dependencies(${TARGET} ${INTERMEDIATE_TARGET})
    endforeach()
endfunction()
