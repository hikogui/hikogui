# Copyright Take Vos 2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_custom_target(codec_example_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    hikogui
)

add_executable(json_to_bon8 WIN32 MACOSX_BUNDLE)
target_sources(json_to_bon8 PRIVATE json_to_bon8.cpp)
target_link_libraries(json_to_bon8 PRIVATE hikogui)
target_include_directories(json_to_bon8 PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(json_to_bon8 codec_example_resources)

#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------

install(TARGETS json_to_bon8 DESTINATION examples/codec)

# copy additional "hikogui library" resources from top-level
install(DIRECTORY ../../resources/  DESTINATION examples/codec/resources)

# copy vcpkg dependencies (experimental)
if (VCPKG_TOOLCHAIN)
    x_vcpkg_install_local_dependencies(TARGETS hikogui_demo DESTINATION examples/codec)
endif()

