function(target_add_icon TARGET ICON)
    find_package(python COMPONENTS Interpreter)

    set(SVG2TTICON_PY ${CMAKE_SOURCE_DIR}/Utilities/SVG2TTIcon.py)

	# All shaders for a sample are found here.
	set(current-input-path ${CMAKE_CURRENT_SOURCE_DIR}/${ICON})

	get_filename_component(icon-filename ${ICON} NAME)
	set(current-intermediate-path ${CMAKE_CURRENT_BINARY_DIR}/${icon-filename}.tticon)

	set(current-output-path ${CMAKE_CURRENT_BINARY_DIR}/${icon-filename}.tticon.hpp)

	# Add a custom command to compile GLSL to SPIR-V.
	add_custom_command(
		OUTPUT ${current-intermediate-path}
		COMMAND ${Python_EXECUTABLE} ${SVG2TTICON_PY} -o ${current-intermediate-path} ${current-input-path}
		DEPENDS ${current-input-path}
		IMPLICIT_DEPENDS CXX ${current-input-path}
		VERBATIM)

	add_custom_command(
		OUTPUT ${current-output-path}
		COMMAND BinaryToHPP ${current-intermediate-path} ${current-output-path}
		DEPENDS ${current-intermediate-path} BinaryToHPP
		IMPLICIT_DEPENDS CXX ${current-intermediate-path}
		VERBATIM
	)

	# Make sure our native build depends on this output.
	set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
	target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(target_add_icon)
