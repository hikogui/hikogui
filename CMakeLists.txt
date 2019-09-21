
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0074 NEW)
enable_testing()

project(TTauri CXX)
 

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")
include(AddIcon)
include(AddStaticResource)

if (MSVC)
    add_compile_options(/wd4068 /nologo)
    add_link_options(/nologo)
	add_definitions(-DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)

    # std::result_of used by fmt.
    add_definitions(-D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)

    if (NOT (${CMAKE_CXX_COMPILER_ID} MATCHES Clang))
        if (NOT (${CMAKE_BUILD_TYPE} MATCHES Debug))
            # Whole program optimization.
            add_compile_options(-GL)
            add_link_options(-LTCG)
        endif()
    endif()
endif ()

add_subdirectory(ThirdPartyLibraries)
add_subdirectory(Utilities)
add_subdirectory(Required)
add_subdirectory(Time)
add_subdirectory(Diagnostic)
add_subdirectory(Foundation)
add_subdirectory(Config)
add_subdirectory(Audio)
add_subdirectory(Draw)
add_subdirectory(GUI)
add_subdirectory(Widgets)
add_subdirectory(Application)

add_icon(Icon_objs 
    "Themes/Icons/Application Icon.svg"
    "Themes/Icons/Close Window.svg"
    "Themes/Icons/Maximize Window.svg"
    "Themes/Icons/Minimize Window.svg"
    "Themes/Icons/MultiColor.svg"
)

set(TTauri_resources
    ${Icon_objs}
    Themes/Fonts/Roboto/Roboto-Regular.ttf
)

add_library(TTauri INTERFACE)
target_static_resource(TTauri ${TTauri_resources})
target_link_libraries(TTauri INTERFACE TTauri_Application)



add_subdirectory(TTauriDevelopmentApplication)


#set(TTauri_do_not_analyze
#    ${BISON_TTauriConfigParser_OUTPUTS}
#    ${FLEX_TTauriConfigScanner_OUTPUTS}
#    ${TTauriTests_srcs}
#)

#if(MSVC)
#    add_compile_options(/wd4996 /wd4005)
#    if(TTAURI_ENABLE_CODE_ANALYSIS)
#        set_source_files_properties(${TTauri_do_not_analyze} PROPERTIES COMPILE_FLAGS /analyze-)
#        add_compile_options(/analyze /analyze:plugin EspXEngine.dll)
#        add_compile_options(/wd26447)
#       add_compile_options(/wd26486) # lifetime.3 tooooooo many false positives.
#   endif()
#endif()



