
# Add resources to a target
#
# Synopsys:
#   target_resources(<target> <resource> ...)
#
# Arguments:
#   target - The target to add the resources to
#   <resource> ... - One or more resources to add.
#
function(target_resources target)
    get_target_property(resource_files ${target} RESOURCE)
    if (resource_files STREQUAL "resource_files-NOTFOUND")
        set(resource_files "")
    endif()

    foreach(resource_file IN LISTS ARGN)
        list(APPEND resource_files "${resource_file}")
    endforeach()

    set_target_properties(${target} PROPERTIES RESOURCE "${resource_files}")
endfunction()

# Add resources from a library target to a target.
#
# Synopsys:
#   target_link_resources(<target> <link_target>)
#
# Arguments:
#   target - The target to add the resources to
#   link_target - The target from which to copy the resources.
#
function(target_link_resources target link_target)
    get_target_property(link_resource_files ${link_target} RESOURCE)
    if (link_resource_files STREQUAL "link_resource_files-NOTFOUND")
        set(link_resource_files "")
    endif()

    get_target_property(resource_files ${target} RESOURCE)
    if (resource_files STREQUAL "resource_files-NOTFOUND")
        set(resource_files "")
    endif()

    foreach(resource_file IN LISTS link_resource_files)
        list(APPEND resource_files "${resource_file}")
    endforeach()

    set_target_properties(${target} PROPERTIES RESOURCE "${resource_files}")
endfunction()

# Get a list of resources of a target.
# 
# This function returns a list of resource filenames, optionally prefixed
# with the BASE_DIR argument.
#
# Synopsis:
#   get_target_resources(<target> <out> [ BASE_DIR <directory> ])
#
# Options:
#   target - The target.
#   out - The output variable to put the filenames into.
#
function(get_target_resources target out)
    get_target_property(target_resource_files ${target} RESOURCE)
    set(resource_files "")
    foreach(target_resource_file IN LISTS target_resource_files)
        cmake_path(GET target_resource_file FILENAME filename)
        list(APPEND resource_files "${filename}")
    endforeach()
    set(${out} "${resource_files}" PARENT_SCOPE)
endfunction()
