
function(get_relative_path RET FILE_PATH)
    set(INPUT_RELPATH ${FILE_PATH})

    string(FIND ${FILE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/" IN_SOURCE_DIR)
    if(${IN_SOURCE_DIR} EQUAL 0)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" INPUT_RELPATH ${FILE_PATH})
    endif()

    string(FIND ${FILE_PATH} "${CMAKE_CURRENT_BINARY_DIR}/" IN_BINARY_DIR)
    if(${IN_BINARY_DIR} EQUAL 0)
        string(REPLACE "${CMAKE_CURRENT_BINARY_DIR}/" "" INPUT_RELPATH ${FILE_PATH})
    endif()

    set(${RET} ${INPUT_RELPATH} PARENT_SCOPE)
endfunction()
