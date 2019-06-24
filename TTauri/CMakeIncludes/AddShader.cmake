function(target_add_shader TARGET SHADER)
	# Find glslc shader compiler.
	# On Android, the NDK includes the binary, so no external dependency.
	if(ANDROID)
		file(GLOB glslc-folders ${ANDROID_NDK}/shader-tools/*)
		find_program(GLSLC glslc HINTS ${glslc-folders})
	else()
		find_program(GLSLC glslc)
	endif()

	# All shaders for a sample are found here.
	set(current-input-path ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})

	get_filename_component(shader-filename ${SHADER} NAME)
	set(current-intermediate-path ${CMAKE_CURRENT_BINARY_DIR}/${shader-filename}.spv)

	set(current-output-path ${CMAKE_CURRENT_BINARY_DIR}/${shader-filename}.spv.hpp)

	# Add a custom command to compile GLSL to SPIR-V.
	add_custom_command(
		OUTPUT ${current-intermediate-path}
		COMMAND ${GLSLC} -o ${current-intermediate-path} ${current-input-path}
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
endfunction(target_add_shader)
