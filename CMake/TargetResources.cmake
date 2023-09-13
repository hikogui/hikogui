

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
