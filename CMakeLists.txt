
cmake_minimum_required(VERSION 3.19)
enable_testing()

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

if(APPLE)
project(TTauri LANGUAGES CXX OBJC)
else()
project(TTauri LANGUAGES CXX)
endif()

find_package(GTest)
find_package(Python COMPONENTS Interpreter)
find_package(Vulkan REQUIRED)
find_package(Doxygen)

if (DOXYGEN_EXECUTABLE)
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
endif()

if(WIN32)
    if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
        set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)
    endif()
else(APPLE)
    # Unable to use precompiled headers with objc right now
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include(AddStaticResource)
include(AddShader)
include(AddStaticResource)

add_subdirectory(external)
add_subdirectory(tools)

add_library(ttauri STATIC)
add_executable(ttauri_tests)
add_subdirectory(src)

target_link_libraries(ttauri PUBLIC fmt date date-tz ${Vulkan_LIBRARIES} VMA RenderDoc)
target_include_directories(ttauri PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(ttauri PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(ttauri PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
target_include_directories(ttauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(ttauri PUBLIC cxx_std_20)

add_custom_command(
  TARGET ttauri POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tzdata 
    ${CMAKE_BINARY_DIR}/data/tzdata)

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
        # The TTauri generic tokenizer uses large constexpr tables.
        target_compile_options(ttauri PUBLIC -fconstexpr-steps=100000000)

        # Tune according to a more modern CPU.
        target_compile_options(ttauri PUBLIC -mtune=skylake)
    endif()

elseif (MSVC)
    # Turn on a lot of warnings by default.
    target_compile_options(ttauri PUBLIC -W4)

    # Set the compiler to C++20 standard conforming as much as possible.
    target_compile_options(ttauri PUBLIC -permissive-)

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

    # The TTauri generic tokenizer uses large constexpr tables.
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

    if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
        # Enable Security Development Lifecycle checks.
        # This includes run-time checks, don't include this in the Release type
        # Do include it with ReleaseWithDebug.

        # -sdl forces deprecated warnings as errors so also not useful during development.
        #target_compile_options(ttauri PUBLIC -sdl)

        # Just my code
        target_compile_options(ttauri PUBLIC -JMC)
    endif()
endif()

if (WIN32)
    if(NOT TTAURI_ENABLE_CODE_ANALYSIS)
    target_precompile_headers(ttauri PRIVATE <Windows.h>)
    endif()

    # Add vulkan win32 surface support.
    target_compile_definitions(ttauri PUBLIC -DVK_USE_PLATFORM_WIN32_KHR)

    target_link_libraries(ttauri PUBLIC UxTheme.lib)

    target_link_libraries(ttauri PUBLIC synchronization windowsapp.lib)

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

elseif (APPLE)
    # Add vulkan apple's Metal surface support.
    target_compile_definitions(ttauri PUBLIC -DVK_USE_PLATFORM_METAL_EXT)

    target_link_libraries(ttauri PUBLIC "-framework Foundation -framework AppKit")
endif()

add_shader(ttauri_binary_resources
    src/TTauri/GUI/pipeline_image.vert
    src/TTauri/GUI/pipeline_image.frag
    src/TTauri/GUI/pipeline_flat.vert
    src/TTauri/GUI/pipeline_flat.frag
    src/TTauri/GUI/pipeline_box.vert
    src/TTauri/GUI/pipeline_box.frag
    src/TTauri/GUI/pipeline_SDF.vert
    src/TTauri/GUI/pipeline_SDF.frag
    src/TTauri/GUI/pipeline_tone_mapper.vert
    src/TTauri/GUI/pipeline_tone_mapper.frag
)
set(ttauri_binary_resources ${ttauri_binary_resources}
    data/elusiveicons-webfont.ttf
    data/ttauri_icons.ttf
)

target_static_resource(ttauri ${ttauri_binary_resources})

add_custom_command(
    TARGET ttauri POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_BINARY_DIR}/data
)

if(NOT TTAURI_ENABLE_CODE_ANALYSIS)
target_precompile_headers(ttauri_tests PRIVATE <gtest/gtest.h>)
endif()

target_include_directories(ttauri_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ttauri_tests PRIVATE gtest_main ttauri)
gtest_discover_tests(ttauri_tests)
add_custom_command(
    TARGET ttauri_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/data
        ${CMAKE_CURRENT_BINARY_DIR}
)

set(TTauri_do_not_analyze
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

if(MSVC)
    if(TTAURI_ENABLE_CODE_ANALYSIS)
        set_source_files_properties(${TTauri_do_not_analyze} PROPERTIES COMPILE_FLAGS /analyze-)
        target_compile_options(ttauri PUBLIC -analyze)
        #target_compile_options(ttauri PUBLIC /analyze:plugin EspXEngine.dll)
   endif()
endif()



