
#-------------------------------------------------------------------
# Build Target: ttauri_demo                             (executable)
#-------------------------------------------------------------------

add_executable(ttauri_demo WIN32 MACOSX_BUNDLE)

target_link_libraries(ttauri_demo PRIVATE ttauri)

add_subdirectory(src)

add_custom_target(ttauri_demo_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    ttauri
    resources/ttauri_demo.png
    resources/locale/en.po
    resources/locale/nl.po
)

add_dependencies(ttauri_demo ttauri_demo_resources)

set(ttauri_demo_do_no_analyze
    src/preferences_controller.cpp
    src/main_window_controller.cpp
    src/audio_preferences_controller.cpp
    src/main.cpp
    src/application_controller.cpp
    src/license_preferences_controller.cpp
)

if(MSVC)
    if(TTAURI_ENABLE_CODE_ANALYSIS)
        set_source_files_properties(${ttauri_demo_do_no_analyze} PROPERTIES COMPILE_FLAGS -analyze-)
    endif()
endif()
