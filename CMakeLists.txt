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
# Setup CMake Includes
#-------------------------------------------------------------------

include(AddShader)
include(ShowBuildTargetProperties)
include(FetchContent)

# disable adding CTest build targets like "NightlyStart" (before ctest include)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)

#-------------------------------------------------------------------
# Find Dependencies
#-------------------------------------------------------------------

#
# GoogleTest - non-vcpkg, directly build from externals
#
if(BUILD_TESTING)
    set(INSTALL_GTEST OFF CACHE INTERNAL "Don't install gtest")
    set(BUILD_GMOCK OFF CACHE INTERNAL "Don't build gmock")
    FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG release-1.11.0)
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)
    add_executable(hikogui_tests)
    # PRE_TEST delays test discovery until just prior to test execution
    gtest_discover_tests(hikogui_tests DISCOVERY_MODE PRE_TEST)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_target_properties(gtest      PROPERTIES FOLDER extern)
    set_target_properties(gtest_main PROPERTIES FOLDER extern)
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
set_target_properties(hikogui PROPERTIES DEBUG_POSTFIX "d")

#-------------------------------------------------------------------
# Setup Sources
#-------------------------------------------------------------------

# This needs to be below the build target commands (add_*).
# We omit adding source files to the `add_*` commands, but add them
# later using target_sources() in CMakeLists of subdirectories.

add_subdirectory(src/hikogui)

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

add_custom_target(hikogui_shaders)
add_shader_target(hikogui_shaders
    shaders/utils.glsl
    shaders/pipeline_image.vert
    shaders/pipeline_image.frag
    shaders/pipeline_box.vert
    shaders/pipeline_box.frag
    shaders/pipeline_SDF.vert
    shaders/pipeline_SDF.frag
    shaders/pipeline_alpha.vert
    shaders/pipeline_alpha.frag
    shaders/pipeline_tone_mapper.vert
    shaders/pipeline_tone_mapper.frag
)

set(hikogui_resource_files
    resources/themes/hikogui_dark.theme.json
    resources/themes/hikogui_light.theme.json
    resources/fonts/elusiveicons-webfont.ttf
    resources/fonts/hikogui_icons.ttf
    resources/win32.keybinds.json
)

add_custom_target(hikogui_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders
    DEPENDS ${hikogui_resource_files} hikogui_shaders)

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
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources/
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

if (BUILD_TESTING)
    target_link_libraries(hikogui_tests PRIVATE gtest_main hikogui)
    target_include_directories(hikogui_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})


    add_custom_target(hikogui_tests_resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/data ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_dependencies(hikogui_tests hikogui_tests_resources)

    #-------------------------------------------------------------------
    # Installation Rules: hikogui_tests                      (executable)
    #-------------------------------------------------------------------
    install(TARGETS hikogui_tests DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
    install(DIRECTORY tests/data/ DESTINATION tests COMPONENT tests EXCLUDE_FROM_ALL)
endif()

#-------------------------------------------------------------------
# Build examples
#-------------------------------------------------------------------
add_custom_target(examples)
add_subdirectory(examples/codec)
add_subdirectory(examples/custom_widgets)
add_subdirectory(examples/hikogui_demo)
add_subdirectory(examples/vulkan/triangle)
add_subdirectory(examples/widgets)

#-------------------------------------------------------------------
# Display Compiler and Linker properties of Build Targets
#-------------------------------------------------------------------

show_build_target_properties(hikogui)
show_build_target_properties(hikogui_tests)

#-------------------------------------------------------------------
# Build Documentation
#-------------------------------------------------------------------

#
# Doxygen
#
find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_EXCLUDE_PATTERNS *_tests.cpp *.cpp *.mm)
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

    doxygen_add_docs(docs src/hikogui docs)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share COMPONENT docs EXCLUDE_FROM_ALL)
else()
    message("Please install Doxygen to generate the documentation.")
endif()
