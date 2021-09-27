
#-------------------------------------------------------------------
# Build Target: ttauri_demo                             (executable)
#-------------------------------------------------------------------

add_custom_target(codec_example_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    ttauri
)

add_executable(json_to_bon8 WIN32 MACOSX_BUNDLE)
target_sources(json_to_bon8 PRIVATE json_to_bon8.cpp)
target_link_libraries(json_to_bon8 PRIVATE ttauri)
target_include_directories(json_to_bon8 PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(json_to_bon8 codec_example_resources)

#-------------------------------------------------------------------
# Installation Rules: ttauri_demo
#-------------------------------------------------------------------

install(TARGETS json_to_bon8 DESTINATION examples/codec)

# copy additional "ttauri library" resources from top-level
install(DIRECTORY ../../resources/  DESTINATION examples/codec/resources)

# copy vcpkg dependencies (experimental)
if (VCPKG_TOOLCHAIN)
    x_vcpkg_install_local_dependencies(TARGETS ttauri_demo DESTINATION examples/codec)
endif()

