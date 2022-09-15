# Copyright Jens A. Koch 2021-2022.
# Copyright Take Vos 2019-2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.19)

#-------------------------------------------------------------------
# Configure early CMAKE paths
#-------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include(SetupVcpkg)
include(SetupCompiler)

#-------------------------------------------------------------------
# Build Options
#-------------------------------------------------------------------

option(BUILD_SHARED_LIBS    "Build shared libraries"            OFF)
option(HI_ENABLE_ANALYSIS   "Compile using -analyze"            OFF)
option(HI_ENABLE_ASAN       "Compile using address sanitizer"   OFF)

include(CMakeDependentOption)
cmake_dependent_option(HI_BUILD_EXAMPLES "Build example applications"   OFF "NOT HI_ENABLE_ANALYSIS" ON)
cmake_dependent_option(HI_BUILD_TESTS    "Build tests"                  ON  "NOT HI_ENABLE_ANALYSIS" OFF)
cmake_dependent_option(HI_INSTALL        "Generate installation target" ON  "NOT HI_ENABLE_ANALYSIS" OFF)
cmake_dependent_option(HI_BUILD_PCH      "Build precompiled headers"    ON  "NOT HI_ENABLE_ANALYSIS" OFF)

#-------------------------------------------------------------------
# Project
#-------------------------------------------------------------------

if(APPLE)
    set(LANGUAGES CXX OBJCXX)
else()
    set(LANGUAGES CXX)
endif()

# vcpkg.json is the primary source for version data
file(READ ${CMAKE_SOURCE_DIR}/vcpkg.json VCPKG_JSON_STRING)
string(JSON HI_LIB_NAME     GET ${VCPKG_JSON_STRING} "name")
string(JSON HI_LIB_VERSION  GET ${VCPKG_JSON_STRING} "version")
string(JSON HI_LIB_LICENSE  GET ${VCPKG_JSON_STRING} "license")
string(JSON HI_LIB_DESC     GET ${VCPKG_JSON_STRING} "description")
string(JSON HI_LIB_HOMEPAGE GET ${VCPKG_JSON_STRING} "homepage")

configure_file("package.json.in" "package.json" @ONLY)
file(READ ${CMAKE_CURRENT_BINARY_DIR}/package.json PACKAGE_JSON_STRING)
string(JSON HI_LIB_DISPLAY_NAME GET ${PACKAGE_JSON_STRING} "display-name")
string(JSON HI_LIB_VENDOR GET ${PACKAGE_JSON_STRING} "vendor")

project(${HI_LIB_NAME} VERSION ${HI_LIB_VERSION} LANGUAGES ${LANGUAGES})

#-------------------------------------------------------------------
# Define Constants
#-------------------------------------------------------------------

set(HI_WIN32 0)
set(HI_MACOS 0)
set(HI_POSIX 0)
set(HI_X64 0)

if (APPLE)
    set(HI_MACOS 1)
    set(HI_POSIX 1)
elseif (WIN32)
    set(HI_WIN32 1)
endif()

set(x64_list x86 X86 amd64 AMD64)
if (${CMAKE_SYSTEM_PROCESSOR} IN_LIST x64_list)
    set(HI_X64 1)
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
include(ShowBuildTargetProperties)
include(FetchContent)

#-------------------------------------------------------------------
# Find Dependencies
#-------------------------------------------------------------------

#
# GoogleTest - non-vcpkg, directly build from externals
#
if(HI_BUILD_TESTS)
    set(INSTALL_GTEST OFF CACHE INTERNAL "Don't install gtest")
    set(BUILD_GMOCK OFF CACHE INTERNAL "Don't build gmock")
    FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG release-1.11.0)
    FetchContent_MakeAvailable(googletest)
endif()

#
# Vulkan SDK Headers
#
if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR
        "Please set the VULKAN_SDK environment variable to the directory in which the SDK is installed.\n"
        "It looks like C:\\VulkanSDK\\1.0.0.0, but with the version number being the version that was installed.\n"
        "set VULKAN_SDK=\"C:\\VulkanSDK\\1.0.0.0\"\n")
endif()
find_package(Vulkan REQUIRED)

#
# Vulkan Memory Allocator - installed as part of Vulkan SDK headers
#
if(NOT EXISTS "${Vulkan_INCLUDE_DIRS}/vma")
    message(FATAL_ERROR "Please make sure to include VMA (Vulkan Memory Allocator) with the installation of VulkanSDK.\n")
endif()

#-------------------------------------------------------------------
# Setup Build Targets
#-------------------------------------------------------------------

add_library(hikogui)
add_library(hikogui::hikogui ALIAS hikogui)

if(HI_BUILD_TESTS)
    add_executable(hikogui_tests)
endif()

#-------------------------------------------------------------------
# Setup Sources
#-------------------------------------------------------------------

# This needs to be below the build target commands (add_*).
# We omit adding source files to the `add_*` commands, but add them
# later using target_sources() in CMakeLists of subdirectories.

add_subdirectory(src/hikogui)
add_subdirectory(tools)

#-------------------------------------------------------------------
# Build Target: hikogui                                     (library)
#-------------------------------------------------------------------

target_compile_features(hikogui PUBLIC cxx_std_20)

set_target_properties(hikogui PROPERTIES VERSION ${HIKOGUI_PROJECT_VERSION_SHORT} SOVERSION ${HIKOGUI_MAJOR_VERSION})
set_target_properties(hikogui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(APPLE)
    target_link_libraries(hikogui PUBLIC "-framework Foundation -framework AppKit")
endif()
if(WIN32)
	target_link_libraries(hikogui PUBLIC "ws2_32")
	target_link_libraries(hikogui PUBLIC "DXGI")
	target_link_libraries(hikogui PUBLIC "bcrypt")
	target_link_libraries(hikogui PUBLIC "winmm")
	target_link_libraries(hikogui PUBLIC "dwmapi")
endif()

# Add the Vulkan libraries.
target_link_libraries(hikogui PUBLIC ${Vulkan_LIBRARIES})
target_include_directories(hikogui PUBLIC ${Vulkan_INCLUDE_DIRS})
if (WIN32)
    # Add vulkan win32 surface support.
    target_compile_definitions(hikogui PUBLIC -DVK_USE_PLATFORM_WIN32_KHR)

    # vulkan.h will include the windows headers, so we must define all sorts of stuff on the command line.
    target_compile_definitions(hikogui PUBLIC -DUNICODE)
    target_compile_definitions(hikogui PUBLIC -D_UNICODE)
    target_compile_definitions(hikogui PUBLIC -D_CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(hikogui PUBLIC -DNOMINMAX)
    target_compile_definitions(hikogui PUBLIC -DWIN32_LEAN_AND_MEAN)

    # Minimum Windows 10.
    target_compile_definitions(hikogui PUBLIC -DWINVER=0x0a00)
    target_compile_definitions(hikogui PUBLIC -D_WIN32_WINNT=0x0a00)

elseif (APPLE)
    # Add vulkan apple's Metal surface support.
    target_compile_definitions(hikogui PUBLIC -DVK_USE_PLATFORM_METAL_EXT)
endif()


target_include_directories(hikogui PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:include>"
)
target_include_directories(hikogui PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(hikogui PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src)

#
# We will support the following CPUs:
#   * Intel Ivy Bridge from 2012, still used by Mac Pro sold in 2019.
#   * AMD Steamroller from 2014
#
if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    # Set the execution character encoding to UTF-8
    if (MSVC)
        target_compile_options(hikogui PUBLIC -utf-8)
    else()
        target_compile_options(hikogui PUBLIC -fexec-charset=UTF-8)
    endif()

    # Clang will complain about __builtin_assume even on constexpr functions and operators,
    # this warning is absolutely useless.
    target_compile_options(hikogui PUBLIC -Wno-assume)

    # Clang will complain about calling hidden-friend-template-functions.
    target_compile_options(hikogui PUBLIC -Wno-unknown-warning-option -Wno-c++20-extensions -Wno-c++2a-extensions)

    # vk_mem_alloc.h seems to turn on nullability completeness checks on clang.
    # It seems this check should only be used for interopability with swift
    target_compile_options(hikogui PUBLIC -Wno-nullability-completeness)

    # IvyBridge:                     F16C             FSGSBASE, AVX,           AES, PCLMUL        MMX, SSE, SSE2, SSE3,        SSSE3, SSE4.1, SSE4.2,     POPCNT, RDRND
    # Steamroller 0x15-v3: BMI, TBM, F16C, FMA, FMA4, FSGSBASE, AVX, XOP, LWP, AES, PCLMUL, CX16, MMX, SSE, SSE2, SSE3, SSE4A, SSSE3, SSE4.1, SSE4.2, ABM
    # POPCNT is supported by Steamroller through he SSE4A instruction set.
    target_compile_options(hikogui PUBLIC -mf16c -mfsgsbase -mavx -maes -mpclmul -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)

    # The Microsoft version of clang does not implement all clang command line arguments.
    if (NOT MSVC)
        # The hikogui generic tokenizer uses large constexpr tables.
        target_compile_options(hikogui PUBLIC -fconstexpr-steps=100000000)

        # Tune according to a more modern CPU.
        target_compile_options(hikogui PUBLIC -mtune=skylake)
    endif()

elseif (MSVC)

    # Turn on a lot of warnings by default.
    target_compile_options(hikogui PUBLIC -W4)

    # Set the compiler to C++20 standard conforming as much as possible.
    target_compile_options(hikogui PUBLIC -permissive-)

    # suppress msbuild startup banner
    target_compile_options(hikogui PUBLIC -nologo)

    # By default MSVC reports a incorrect version in __cplusplus
    # This option will make it work correctly.
    target_compile_options(hikogui PUBLIC -Zc:__cplusplus)

    # Use the C++20 compliant preprocessor, which include __VA_OPT__()
    # This causes warning "C5105: macro expansion producing 'defined' has undefined behavior" in winbase.h
    # which is supposed to be fixed in a newer version of the SDK.
    target_compile_options(hikogui PUBLIC -Zc:preprocessor)
    target_compile_options(hikogui PUBLIC -wd5105)

    # Set the execution and source character encoding to UTF-8
    target_compile_options(hikogui PUBLIC -utf-8)

    # Support Intel's SkyLake and AMD's Excavator.
    # see hardware_support.md
    target_compile_options(hikogui PUBLIC -arch:AVX)

    # The hikogui generic tokenizer uses large constexpr tables.
    target_compile_options(hikogui PUBLIC "-constexpr:steps100000000")

    # C4068: unknown pragma.
    # Ignore unknown pragmas, needed for other compilers.
    target_compile_options(hikogui PUBLIC -wd4068)

    # C4324: structure was padded due to alignment specifier.
    # Of course it is, that is what the alignment specifier is used for??????
    target_compile_options(hikogui PUBLIC -wd4324)

    # C4100: unreferenced formal parameter.
    # This warning is in the way when you want to document a parameter of a virtual function
    # with an implementation that does not use the parameter.
    target_compile_options(hikogui PUBLIC -wd4100)

    # C4127: conditional expression is constant.
    # False positive with expressions that include template parameters.
    target_compile_options(hikogui PUBLIC -wd4127)

    # C6326: Potential comparison of a constant with another constant.
    # False positive in constexpr context.
    target_compile_options(hikogui PUBLIC -wd6326)

    # C6239: (<non-zero constant> && <expression>) always evaluates to the result of <expression>. Did you intend to use the bitwise-and operator?"
    # False positive when using logical operators in constexpr context.
    target_compile_options(hikogui PUBLIC -wd6239)

    # C6262: Function uses 'x' bytes of stack: exceeds /analyze:stacksize 'x'. Consider moving some data to heap.
    # False positives when returning data from a function in constexpr context.
    target_compile_options(hikogui PUBLIC -wd6262)

    # C4505: unreferenced local function has been removed.
    # False positive when calling a local function from within a "if constexpr" block.
    target_compile_options(hikogui PUBLIC -wd4505)

    # C4648: standard attribute 'no_unique_address' is ignored
    target_compile_options(hikogui PUBLIC -wd4648)

    # C4744: 'x' has different type in 'foo.cpp' and 'bar.cpp':
    #        'struct (8 bytes)' and '__declspec(align(8)) struct (8 bytes)' [foobar.vcxproj]
    # Bug in linker where atomic variables defined in a header file and used in two .cpp files be
    # interpreted as different types. Microsoft knows about the bug for many years.
    target_link_options(hikogui PUBLIC "/ignore:4744")

    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        # Enable Security Development Lifecycle checks.
        # This includes run-time checks, don't include this in the Release type
        # Do include it with ReleaseWithDebug.

        # -sdl forces deprecated warnings as errors so also not useful during development.
        #target_compile_options(hikogui PUBLIC -sdl)

        # Just my code enables the VS debugger to step over system, framework, library, and other non-user calls.
        target_compile_options(hikogui PUBLIC -JMC)
    endif()

    if(HI_ENABLE_ASAN)
        target_compile_options(hikogui PUBLIC -fsanitize=address)
    endif()

    if(HI_ENABLE_ANALYSIS)
        target_compile_options(hikogui PUBLIC -analyze)

        #target_compile_options(hikogui PUBLIC -analyze:ruleset ${CMAKE_CURRENT_SOURCE_DIR}/AllRules.ruleset)

        # The Core Guidelines checker, not so useful for real programs.
        message(NOTICE "The following environment variables need to be set for the core guidelines checker to work:")
        message(NOTICE "    esp.extensions=cppcorecheck.dll")
        message(NOTICE "    esp.annotationbuildlevel=ignore")
        message(NOTICE "    CAExcludePath=C:\\VulkanSDK;C:\\Program Files")
        message(NOTICE "")
        target_compile_options(hikogui PUBLIC -analyze:plugin EspXEngine.dll)

        # C26440: Function '...' can be declared 'noexcept' (f.6).
        # It requires lambda function to also be declared 'noexcept' that makes the code messy.
        target_compile_options(hikogui PUBLIC -wd26440)

        # C26446: Prefer to use gsl::at() instead of unchecked subscript operator (bounds.4).
        # This rule makes applications slow.
        target_compile_options(hikogui PUBLIC -wd26446)

        # C26447: The function is declared 'noexcept' but calls function '...' which may throw exceptions (f.6).
        # Since c++20 std::terminate will be called on throw.
        target_compile_options(hikogui PUBLIC -wd26447)

        # C26481: Don't use pointer arithmetic. Use span instead (bounds.1).
        # False positive everywhere std::format() is used.
        target_compile_options(hikogui PUBLIC -wd26481)

        # C26482: Only index into arrays using constant expressions (bounds.2).
        # This rule makes it so you can not use a variable to index into an array.
        target_compile_options(hikogui PUBLIC -wd26482)

        # C26821: For '...', consider using gsl::span instead of std::span to guarantee runtime bounds safety (gsl.view).
        # This rule makes applications slow.
        target_compile_options(hikogui PUBLIC -wd26821)

        # C28020: The expression '0<=_Param_(1)&&_Param_(1)<=256-1' is not true at this call.
        # False positives in base_n, where the index is static_cast<uint8_t>().
        # The bug (since 2020) in the compiler seem to have been triggered in unrelated code.
        # https://developercommunity.visualstudio.com/t/c28020-false-positives/923103
        target_compile_options(hikogui PUBLIC -wd28020)

    endif()
endif()

#-------------------------------------------------------------------
# Copy resources needed by executables
#-------------------------------------------------------------------

add_shader(hikogui_shader_objects
    src/hikogui/GFX/utils.glsl
    src/hikogui/GFX/pipeline_image.vert
    src/hikogui/GFX/pipeline_image.frag
    src/hikogui/GFX/pipeline_box.vert
    src/hikogui/GFX/pipeline_box.frag
    src/hikogui/GFX/pipeline_SDF.vert
    src/hikogui/GFX/pipeline_SDF.frag
    src/hikogui/GFX/pipeline_alpha.vert
    src/hikogui/GFX/pipeline_alpha.frag
    src/hikogui/GFX/pipeline_tone_mapper.vert
    src/hikogui/GFX/pipeline_tone_mapper.frag
)

target_static_resource(hikogui
    ${hikogui_shader_objects}
    data/elusiveicons-webfont.ttf
    data/hikogui_icons.ttf
)

add_custom_target(hikogui_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    resources/themes/hikogui_dark.theme.json
    resources/themes/hikogui_light.theme.json
    resources/win32.keybinds.json
)

add_dependencies(hikogui hikogui_resources)

#-------------------------------------------------------------------
# Installation Rules: hikogui                               (library)
#-------------------------------------------------------------------

include(GNUInstallDirs)

install(TARGETS hikogui RESOURCE)

# Copy all header files for the library.
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")

# Copy all resource files needing to be added to the resource directory of an application
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/
        DESTINATION share/hikogui/resources)

# On Windows, the dynamic libs go in the same dir as the executable and static libs go into "lib".
get_target_property(target_type hikogui TYPE)
if(WIN32 AND target_type STREQUAL DYNAMIC_LIBRARY)
    # Install the library and headers.
    install(TARGETS hikogui EXPORT hikogui
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR} # <- .dll next to exe
    )
else()
    # Install the library and headers.
    install(TARGETS hikogui EXPORT hikogui
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # <- .lib
    )
endif()

#export(TARGETS
#        hikogui
#    NAMESPACE hikogui::
#    FILE "${CMAKE_CURRENT_BINARY_DIR}/hikogui-config.cmake"
#)
install(
    EXPORT hikogui
    DESTINATION share/hikogui
    NAMESPACE hikogui::
    FILE "hikoguiTargets.cmake"
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/hikoguiConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/hikoguiConfig.cmake"
    INSTALL_DESTINATION lib/cmake/hikogui
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/hikoguiConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/hikoguiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/hikoguiConfigVersion.cmake"
    DESTINATION share/hikogui
)
#-------------------------------------------------------------------
# Build Target: hikogui_tests                            (executable)
#-------------------------------------------------------------------

if(HI_BUILD_TESTS)

    enable_testing()

    target_link_libraries(hikogui_tests PRIVATE gtest_main hikogui)

    target_include_directories(hikogui_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    if(HI_BUILD_PCH)
        if(NOT HI_ENABLE_ANALYSIS)
            target_precompile_headers(hikogui_tests PRIVATE <gtest/gtest.h>)
        endif()
    endif()

    include(GoogleTest)

    # PRE_TEST delays test discovery until just prior to test execution
    gtest_discover_tests(hikogui_tests DISCOVERY_MODE PRE_TEST)

    add_custom_command(
        TARGET hikogui_tests PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/data
            ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_target_properties(gtest      PROPERTIES FOLDER extern)
    set_target_properties(gtest_main PROPERTIES FOLDER extern)
endif()

#-------------------------------------------------------------------
# Installation Rules: hikogui_tests                      (executable)
#-------------------------------------------------------------------
if(HI_BUILD_TESTS)
    install(TARGETS hikogui_tests DESTINATION tests)
    install(DIRECTORY tests/data/ DESTINATION tests)
    if (BUILD_SHARED_LIBS)
        x_vcpkg_install_local_dependencies(TARGETS hikogui_tests DESTINATION tests)
    endif()
endif()

#-------------------------------------------------------------------
# Build examples
#-------------------------------------------------------------------
if(HI_BUILD_EXAMPLES)
    add_subdirectory(examples/hikogui_demo)
    add_subdirectory(examples/widgets)
    add_subdirectory(examples/custom_widgets)
    add_subdirectory(examples/codec)
else()
    # By default examples marked with EXCLUDE_FROM_ALL will not be build by
    # `make all`, which is also what Visual Studio uses to improve compilation speed.
    add_subdirectory(examples/hikogui_demo EXCLUDE_FROM_ALL)
    add_subdirectory(examples/widgets EXCLUDE_FROM_ALL)
    add_subdirectory(examples/custom_widgets EXCLUDE_FROM_ALL)
    add_subdirectory(examples/codec EXCLUDE_FROM_ALL)
endif()

#-------------------------------------------------------------------
# Display Compiler and Linker properties of Build Targets
#-------------------------------------------------------------------

show_build_target_properties(hikogui)

if(HI_BUILD_EXAMPLES)
    show_build_target_properties(hikogui_demo)
endif()

if(HI_BUILD_TESTS)
    show_build_target_properties(hikogui_tests)
endif()

#-------------------------------------------------------------------
# Build Documentation
#-------------------------------------------------------------------

#
# Doxygen
#
find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_EXCLUDE_PATTERNS *_tests.cpp *.cpp)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_QUIET YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_WARN_NO_PARAMDOC NO)
    set(DOXYGEN_STRIP_FROM_PATH src)
    set(DOXYGEN_STRIP_FROM_INC_PATH src)

    # Use SHORT_NAMES to prevent Doxygen from generating filenames with double-quotes.
    # ALLOW_UNICODE_NAMES does not escape double-quotes.
    set(DOXYGEN_SHORT_NAMES YES)

    set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
    set(DOXYGEN_ALWAYS_DETAILED_SEC YES)
    set(DOXYGEN_DISTRIBUTE_GROUP_DOC YES)

    set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    #set(DOXYGEN_CLANG_ASSISTED_PARSING YES)
    #set(DOXYGEN_CLANG_OPTIONS "-std=c++20")

    set(DOXYGEN_TAGFILES "${CMAKE_SOURCE_DIR}/docs/media/style/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/")
    set(DOXYGEN_IMAGE_PATH "${CMAKE_SOURCE_DIR}/src/" "${CMAKE_SOURCE_DIR}/docs/")
    set(DOXYGEN_EXAMPLE_PATH "${CMAKE_SOURCE_DIR}/examples/")

    set(DOXYGEN_LAYOUT_FILE "${CMAKE_SOURCE_DIR}/docs/media/style/DoxygenLayout.xml")
    set(DOXYGEN_HTML_COLORSTYLE_HUE 24)
    set(DOXYGEN_HTML_COLORSTYLE_SAT 150)
    set(DOXYGEN_HTML_COLORSTYLE_GAMMA 80)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET "${CMAKE_SOURCE_DIR}/docs/media/style/customdoxygen.css")


    # The following 4 settings are to get protected members to behave as private.
    set(DOXYGEN_ENABLE_PREPROCESSING YES)
    set(DOXYGEN_MACRO_EXPANSION YES)
    set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
    set(DOXYGEN_PREDEFINED "protected=private")

    doxygen_add_docs(doc src/hikogui docs)
else()
    message("Please install Doxygen to generate the documentation.")
endif()
