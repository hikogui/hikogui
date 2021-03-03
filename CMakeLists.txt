
cmake_minimum_required(VERSION 3.19)

#-------------------------------------------------------------------
# Configure early CMAKE paths
#-------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include(SetupVcpkg)

#-------------------------------------------------------------------
# Build Options
#-------------------------------------------------------------------

option(BUILD_SHARED_LIBS    "Build shared libraries"            OFF)
option(TT_BUILD_EXAMPLES    "Build example applications"         ON)
option(TT_BUILD_TESTS       "Build tests"                        ON)
option(TT_BUILD_PCH         "Build precompiled headers"          ON)
option(TT_INSTALL           "Generate installation target"       ON)
option(TT_ENABLE_ANALYSIS   "Compile using -analyze"            OFF)

#-------------------------------------------------------------------
# Compiler Setup
#-------------------------------------------------------------------

# Define C++ Standard to use
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(WIN32)
    # Build for a Windows 10 host system.
    set(CMAKE_SYSTEM_VERSION 10.0)

    message(STATUS "[INFO] BUILD_SHARED_LIBS -> '${BUILD_SHARED_LIBS}'.")

    # When we build statically (MT):
    if(NOT BUILD_SHARED_LIBS)
        # Select MSVC runtime based on CMAKE_MSVC_RUNTIME_LIBRARY.
        # We switch from the multi-threaded dynamically-linked library (default)
        # to the multi-threaded statically-linked runtime library.
        cmake_policy(SET CMP0091 NEW)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()


    # Global Compiler flags for optimized Release and RelWithDebugInfo builds
    # All overrides are for multi-threaded dynamically-linked libs: MD + MDd.
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
        set(CMAKE_USER_MAKE_RULES_OVERRIDE "CMakeOverride.txt")
    endif()

endif()

#-------------------------------------------------------------------
# Project
#-------------------------------------------------------------------

if(APPLE)
    set(LANGUAGES CXX OBJCXX)
else()
    set(LANGUAGES CXX)
endif()
project(ttauri VERSION 0.3.0 LANGUAGES ${LANGUAGES})

#-------------------------------------------------------------------
# Define Constants
#-------------------------------------------------------------------

set(TT_WIN32 0)
set(TT_MACOS 0)
set(TT_POSIX 0)
set(TT_X64 0)

if (APPLE)
    set(TT_MACOS 1)
    set(TT_POSIX 1)
elseif (WIN32)
    set(TT_WIN32 1)
endif()

set(x64_list x86 X86 amd64 AMD64)
if (${CMAKE_SYSTEM_PROCESSOR} IN_LIST x64_list)
    set(TT_X64 1)
endif()

#-------------------------------------------------------------------
# Precompiled Headers (PCH) Configuration
#-------------------------------------------------------------------

if(WIN32)
    if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
        set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)
    endif()
else(APPLE)
    # Unable to use precompiled headers with objc right now
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)
endif()

#-------------------------------------------------------------------
# Setup CMake Includes
#-------------------------------------------------------------------

include(AddStaticResource)
include(AddShader)
include(AddStaticResource)
include(ShowBuildTargetProperties)
include(FetchContent)
if(WIN32)
    include(GetWindowsNtVersion)
endif()

#-------------------------------------------------------------------
# Find Dependencies
#-------------------------------------------------------------------

#
# date
#
find_package(date CONFIG REQUIRED)

#
# fmt
#
find_package(fmt CONFIG REQUIRED)

#
# GoogleTest
#
#find_package(GTest)

#
# GoogleTest - non-vcpkg, directly build from externals
#
if(TT_BUILD_TESTS)
    FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG release-1.10.0)
    FetchContent_MakeAvailable(googletest)

endif()

#
# Vulkan Memory Allocator
#
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

#
# Vulkan SDK Headers
#
if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR
        "Please set the VULKAN_SDK environment variable to the directory in which the SDK is installed.\n"
        "It looks like C:\VulkanSDK\1.0.0.0, but with the version number being the version that was installed.\n"
        "set VULKAN_SDK=\"C:\\VulkanSDK\\1.0.0.0\"\n")
endif()
find_package(Vulkan REQUIRED)

#-------------------------------------------------------------------
# Setup Build Targets
#-------------------------------------------------------------------

add_library(ttauri)
add_library(ttauri::tauri ALIAS ttauri)

if(TT_BUILD_TESTS)
    add_executable(ttauri_tests)
endif()

#-------------------------------------------------------------------
# Setup Sources
#-------------------------------------------------------------------

# This needs to be below the build target commands (add_*).
# We omit adding source files to the `add_*` commands, but add them
# later using target_sources() in CMakeLists of subdirectories.

add_subdirectory(src/ttauri)
add_subdirectory(tools)

#-------------------------------------------------------------------
# Build Target: ttauri                                     (library)
#-------------------------------------------------------------------

set_target_properties(ttauri PROPERTIES VERSION ${TTAURI_PROJECT_VERSION_SHORT} SOVERSION ${TTAURI_MAJOR_VERSION})
set_target_properties(ttauri PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_link_libraries(ttauri PUBLIC
    fmt::fmt
    date::date
    date::date-tz
    ${Vulkan_LIBRARIES}
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
)

if(APPLE)
    target_link_libraries(ttauri PUBLIC "-framework Foundation -framework AppKit")
endif()

target_include_directories(ttauri PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(ttauri PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(ttauri PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
target_include_directories(ttauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_features(ttauri INTERFACE cxx_std_20)

#
# We will support the following CPUs:
#   * Intel Ivy Bridge from 2012, still used by Mac Pro sold in 2019.
#   * AMD Steamroller from 2014
#
if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    # Clang will complain about __builtin_assume even on constexpr functions and operators,
    # this warning is absolutely useless.
    target_compile_options(ttauri PUBLIC -Wno-assume)

    # Clang will complain about calling hidden-friend-template-functions.
    target_compile_options(ttauri PUBLIC -Wno-unknown-warning-option -Wno-c++20-extensions -Wno-c++2a-extensions)

    # vk_mem_alloc.h seems to turn on nullability completeness checks on clang.
    # It seems this check should only be used for interopability with swift
    target_compile_options(ttauri PUBLIC -Wno-nullability-completeness)

    # IvyBridge:                     F16C             FSGSBASE, AVX,           AES, PCLMUL        MMX, SSE, SSE2, SSE3,        SSSE3, SSE4.1, SSE4.2,     POPCNT, RDRND
    # Steamroller 0x15-v3: BMI, TBM, F16C, FMA, FMA4, FSGSBASE, AVX, XOP, LWP, AES, PCLMUL, CX16, MMX, SSE, SSE2, SSE3, SSE4A, SSSE3, SSE4.1, SSE4.2, ABM
    # POPCNT is supported by Steamroller through he SSE4A instruction set.
    target_compile_options(ttauri PUBLIC -mf16c -mfsgsbase -mavx -maes -mpclmul -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)

    # The Microsoft version of clang does not implement all clang command line arguments.
    if (NOT MSVC)
        # The ttauri generic tokenizer uses large constexpr tables.
        target_compile_options(ttauri PUBLIC -fconstexpr-steps=100000000)

        # Tune according to a more modern CPU.
        target_compile_options(ttauri PUBLIC -mtune=skylake)
    endif()

elseif (MSVC)
    # Turn on a lot of warnings by default.
    target_compile_options(ttauri PUBLIC -W4)

    # Set the compiler to C++20 standard conforming as much as possible.
    target_compile_options(ttauri PUBLIC -permissive-)

    # suppress msbuild startup banner
    target_compile_options(ttauri PUBLIC -nologo)

    # By default MSVC reports a incorrect version in __cplusplus
    # This option will make it work correctly.
    target_compile_options(ttauri PUBLIC -Zc:__cplusplus)

    # Use the C++20 compliant preprocessor, which include __VA_OPT__()
    # This causes warning "C5105: macro expansion producing 'defined' has undefined behavior" in winbase.h
    # which is supposed to be fixed in a newer version of the SDK.
    target_compile_options(ttauri PUBLIC -Zc:preprocessor)
    target_compile_options(ttauri PUBLIC -wd5105)

    # Allow system headers to be imported.
    target_compile_options(ttauri PUBLIC -experimental:module)

    # Support Intel's IvyBridge and AMD's Steamroller
    target_compile_options(ttauri PUBLIC -arch:AVX)

    # The ttauri generic tokenizer uses large constexpr tables.
    target_compile_options(ttauri PUBLIC -constexpr:steps 100000000)

    # C4068: unknown pragma.
    # Ignore unknown pragmas, needed for other compilers.
    target_compile_options(ttauri PUBLIC -wd4068)

    # C4324: structure was padded due to alignment specifier.
    # Of course it is, that is what the alignment specifier is used for??????
    target_compile_options(ttauri PUBLIC -wd4324)

    # C4100: unreferenced formal parameter.
    # This warning is in the way when you want to document a parameter of a virtual function
    # with an implementation that does not use the parameter.
    target_compile_options(ttauri PUBLIC -wd4100)

    # C4127: conditional expression is constant.
    # False positive with expressions that include template parameters.
    target_compile_options(ttauri PUBLIC -wd4127)

    # C6326: Potential comparison of a constant with another constant.
    # False positive in constexpr context.
    target_compile_options(ttauri PUBLIC -wd6326)

    # C6239: (<non-zero constant> && <expression>) always evaluates to the result of <expression>. Did you intend to use the bitwise-and operator?"
    # False positive when using logical operators in constexpr context.
    target_compile_options(ttauri PUBLIC -wd6239)

    # C6262: Function uses 'x' bytes of stack: exceeds /analyze:stacksize 'x'. Consider moving some data to heap.
    # False positives when returning data from a function in constexpr context.
    target_compile_options(ttauri PUBLIC -wd6262)

    # C4505: unreferenced local function has been removed.
    # False positive when calling a local function from within a "if constexpr" block.
    target_compile_options(ttauri PUBLIC -wd4505)

    # C4648: standard attribute 'no_unique_address' is ignored
    target_compile_options(ttauri PUBLIC -wd4648)

    if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
        # Enable Security Development Lifecycle checks.
        # This includes run-time checks, don't include this in the Release type
        # Do include it with ReleaseWithDebug.

        # -sdl forces deprecated warnings as errors so also not useful during development.
        #target_compile_options(ttauri PUBLIC -sdl)

        # Just my code enables the VS debugger to step over system, framework, library, and other non-user calls.
        target_compile_options(ttauri PUBLIC -JMC)
    endif()
endif()

if (WIN32)
    if(TT_BUILD_PCH)
        if(NOT TT_ENABLE_ANALYSIS)
            target_precompile_headers(ttauri PRIVATE <Windows.h>)
        endif()
    endif()

    # Add vulkan win32 surface support.
    target_compile_definitions(ttauri PUBLIC -DVK_USE_PLATFORM_WIN32_KHR)

    # Set defines to compile a win32 application.
	target_compile_options(ttauri PUBLIC -D_WIN64 -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)

    # std::result_of used by fmt.
    target_compile_options(ttauri PUBLIC -D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)

    # date library uses std::uncaught_exception
    target_compile_options(ttauri PUBLIC -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

    # winrt will include experimental headers that can not be turned off.
    target_compile_options(ttauri PUBLIC -D_SILENCE_CLANG_COROUTINE_MESSAGE)

    # Make sure that Windows.h doesn't include all kind of weird things.
    target_compile_options(ttauri PUBLIC -DNOMINMAX -DWIN32_NO_STATUS)

    # define the version of the windows header files to use
    get_windows_ntversion(winnt_version)
    target_compile_options(ttauri PUBLIC -D_WIN32_WINNT=${winnt_version})

elseif (APPLE)
    # Add vulkan apple's Metal surface support.
    target_compile_definitions(ttauri PUBLIC -DVK_USE_PLATFORM_METAL_EXT)
endif()

#-------------------------------------------------------------------
# Copy resources needed by executables
#-------------------------------------------------------------------

add_shader(ttauri_static_resources
    src/ttauri/GUI/pipeline_image.vert
    src/ttauri/GUI/pipeline_image.frag
    src/ttauri/GUI/pipeline_flat.vert
    src/ttauri/GUI/pipeline_flat.frag
    src/ttauri/GUI/pipeline_box.vert
    src/ttauri/GUI/pipeline_box.frag
    src/ttauri/GUI/pipeline_SDF.vert
    src/ttauri/GUI/pipeline_SDF.frag
    src/ttauri/GUI/pipeline_tone_mapper.vert
    src/ttauri/GUI/pipeline_tone_mapper.frag
)

set(ttauri_static_resources ${ttauri_static_resources}
    data/elusiveicons-webfont.ttf
    data/ttauri_icons.ttf
)

target_static_resource(ttauri ${ttauri_static_resources})

add_custom_target(ttauri_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    resources/tzdata/version
    resources/themes/ttauri_dark.theme.json
    resources/themes/ttauri_light.theme.json
    resources/win32.keybinds.json
)

add_dependencies(ttauri ttauri_resources)

#-------------------------------------------------------------------
# Installation Rules: ttauri                               (library)
#-------------------------------------------------------------------

include(GNUInstallDirs)

if(WIN32)

    install(TARGETS ttauri RESOURCE)

    # Copy all header files for the library.
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp")

    # Copy all resource files needing to be added to the resource directory of an application
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/
            DESTINATION ${CMAKE_INSTALL_PREFIX}/resources)

    # On Windows, the dynamic libs go in the same dir as the executable and static libs go into "lib".
    get_target_property(target_type ttauri TYPE)

    if(target_type STREQUAL STATIC_LIBRARY)

        # Install the library and headers.
        install(TARGETS ttauri EXPORT ttauri
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # <- .lib
        )
    endif()

    if(target_type STREQUAL DYNAMIC_LIBRARY)

        # Install the library and headers.
        install(TARGETS ttauri EXPORT ttauri
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR} # <- .dll next to exe
        )
    endif()

endif()

#-------------------------------------------------------------------
# Build Target: ttauri_tests                            (executable)
#-------------------------------------------------------------------

if(TT_BUILD_TESTS)

    enable_testing()

    target_link_libraries(ttauri_tests PRIVATE gtest_main ttauri)

    target_include_directories(ttauri_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    if(TT_BUILD_PCH)
        if(NOT TT_ENABLE_ANALYSIS)
            target_precompile_headers(ttauri_tests PRIVATE <gtest/gtest.h>)
        endif()
    endif()

    include(GoogleTest)

    # PRE_TEST delays test discovery until just prior to test execution
    gtest_discover_tests(ttauri_tests DISCOVERY_MODE PRE_TEST)

    add_custom_command(
        TARGET ttauri_tests PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/data
            ${CMAKE_CURRENT_BINARY_DIR}
    )

endif()

#-------------------------------------------------------------------
# Installation Rules: ttauri_tests                      (executable)
#-------------------------------------------------------------------

install(TARGETS ttauri_tests DESTINATION tests)

install(DIRECTORY tests/data/ DESTINATION tests)

# experimental
x_vcpkg_install_local_dependencies(TARGETS ttauri_tests DESTINATION tests)

#-------------------------------------------------------------------
# Build examples
#-------------------------------------------------------------------

if(TT_BUILD_EXAMPLES)

    add_subdirectory(examples/ttauri_demo)

endif()

#-------------------------------------------------------------------
# Remove certain files from static analysis
#-------------------------------------------------------------------

if(TT_ENABLE_ANALYSIS)
    if(MSVC)
        set(ttauri_do_not_analyze
            src/ttauri/GUI/pipeline_flat.cpp
            src/ttauri/GUI/gui_window_vulkan.cpp
            src/ttauri/GUI/gui_device_vulkan.cpp
            src/ttauri/GUI/gui_system_vulkan.cpp
            src/ttauri/GUI/gui_system.cpp
            src/ttauri/GUI/gui_system_vulkan_win32.cpp
            src/ttauri/GUI/gui_window.cpp
            src/ttauri/GUI/gui_window_vulkan_win32.cpp
            src/ttauri/GUI/pipeline_flat_device_shared.cpp
            src/ttauri/GUI/pipeline_image.cpp
            src/ttauri/GUI/pipeline_tone_mapper.cpp
            src/ttauri/GUI/pipeline_image_texture_map.cpp
            src/ttauri/GUI/pipeline_box_device_shared.cpp
            src/ttauri/GUI/pipeline_image_device_shared.cpp
            src/ttauri/GUI/pipeline_tone_mapper_device_shared.cpp
            src/ttauri/GUI/pipeline_image_image.cpp
            src/ttauri/GUI/pipeline_SDF_texture_map.cpp
            src/ttauri/GUI/pipeline_SDF_device_shared.cpp
            src/ttauri/GUI/pipeline_SDF_atlas_rect.cpp
            src/ttauri/GUI/pipeline_SDF.cpp
            src/ttauri/GUI/pipeline_vulkan.cpp
            src/ttauri/GUI/pipeline_box.cpp
            src/ttauri/widgets/WindowWidget.cpp
            src/ttauri/widgets/SystemMenuWidget.cpp
            src/ttauri/widgets/grid_layout_widget.cpp
            src/ttauri/widgets/WindowTrafficLightsWidget.cpp
            src/ttauri/widgets/LineInputWidget.cpp
            src/ttauri/widgets/widget.cpp
            src/ttauri/stencils/glyph_stencil.cpp
            src/ttauri/stencils/pixel_map_stencil.cpp
            src/ttauri/stencils/text_stencil.cpp
            src/ttauri/stencils/stencil.cpp
            src/ttauri/application.cpp
        )
        set_source_files_properties(${ttauri_do_not_analyze} PROPERTIES COMPILE_FLAGS /analyze-)
        target_compile_options(ttauri PUBLIC -analyze)
        #target_compile_options(ttauri PUBLIC /analyze:plugin EspXEngine.dll)
   endif()
endif()

#-------------------------------------------------------------------
# Display Compiler and Linker properties of Build Targets
#-------------------------------------------------------------------

show_build_target_properties(ttauri)

if(TT_BUILD_EXAMPLES)
    show_build_target_properties(ttauri_demo)
endif()

if(TT_BUILD_TESTS)
    show_build_target_properties(ttauri_tests)
endif()

#-------------------------------------------------------------------
# Build Documentation
#-------------------------------------------------------------------

#
# Doxygen
#
find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_EXCLUDE_PATTERNS *.cpp)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_XML NO)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_GENERATE_PDF NO)
    set(DOXYGEN_QUIET YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_WARN_NO_PARAMDOC NO)

    set(DOXYGEN_STRIP_FROM_PATH src)
    set(DOXYGEN_STRIP_FROM_INC_PATH src)

    set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
    set(DOXYGEN_ALWAYS_DETAILED_SEC YES)
    set(DOXYGEN_INLINE_INHERITED_MEMB YES)
    set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    set(DOXYGEN_DISTRIBUTE_GROUP_DOC YES)

    set(DOXYGEN_EXTRACT_ALL YES)

    doxygen_add_docs(doc src/ttauri docs)
else()
    message("Please install Doxygen to generate the documentation.")
endif()
