#
# Show Build Target Properties
#
# Usage:
# add_executable(my_exe WIN32 ${APP_SOURCES})
# show_build_target_properties(my_exe)
#

function(show_build_target_property target property)
  get_target_property(values ${target} ${property})
  if(values)
    if(NOT "${values}" STREQUAL "${property}-NOTFOUND")
      message(STATUS "[${target}] ${property}:\n'${values}'\n")
    endif()
  endif()
endfunction()

function(show_build_target_properties target)
  message(STATUS "[INFO] Properties of Build Target '${target}':\n")
  show_build_target_property(${target} INCLUDE_DIRECTORIES)
  show_build_target_property(${target} LINK_LIBRARIES)
  show_build_target_property(${target} LINK_FLAGS)
  show_build_target_property(${target} COMPILE_OPTIONS)
  show_build_target_property(${target} COMPILE_DEFINITIONS)
  show_build_target_property(${target} CMAKE_EXE_LINKER_FLAGS)
endfunction()