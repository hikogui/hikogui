#
# Show Build Target Properties
#
# Usage:
# add_executable(my_exe WIN32 ${APP_SOURCES})
# show_build_target_properties(my_exe)
#

function(show_build_target_property target property)
  if(NOT TARGET ${target})
    message("There is no target named '${target}'.")
    return()
  endif()

  get_target_property(values ${target} ${property})
  if(values)
    if(NOT "${values}" STREQUAL "${property}-NOTFOUND")
      message(STATUS "[${target}] ${property} -> '${values}'")
    endif()
  endif()
endfunction()

function(show_build_target_properties target)
  message(STATUS "|")
  message(STATUS "[INFO] Properties of Build Target '${target}':")
  set(properties
    SOURCE_DIR
    BINARY_DIR
    INCLUDE_DIRECTORIES
    LINK_LIBRARIES
    LINK_FLAGS
    COMPILE_OPTIONS
    COMPILE_DEFINITIONS
    CMAKE_EXE_LINKER_FLAGS
  )
  foreach (prop ${properties})
    show_build_target_property(${target} ${prop})
  endforeach()
endfunction()