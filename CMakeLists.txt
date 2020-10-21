
cmake_minimum_required(VERSION 3.17)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0054 NEW)
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

if(WIN32)
    if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
        set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)
    endif()
else(APPLE)
    # Unable to use precompiled headers with objc right now
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")
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

    # clang bug does not recognize functions without side-effects in __builtin_assume()
    target_compile_options(ttauri PUBLIC -Wno-assume)

    # The Microsoft version of clang does not implement all clang command line arguments.
    if (NOT MSVC)
        # The TTauri generic tokenizer uses large constexpr tables.
        target_compile_options(ttauri PUBLIC -fconstexpr-steps=100000000)

        # Tune according to a more modern CPU.
        target_compile_options(ttauri PUBLIC -mtune=skylake)
    endif()

elseif (MSVC)
    # By default MSVC reports a incorrect version in __cplusplus
    # This option will make it work correctly.
    target_compile_options(ttauri PUBLIC -Zc:__cplusplus)

    # language_win32 requires RT and RT needs specifically the non-c++20
    # experimental-and-incompatible coroutine support.
    target_compile_options(ttauri PUBLIC -await)

    # Support Intel's IvyBridge and AMD's Steamroller
    target_compile_options(ttauri PUBLIC -arch:AVX)

    # The TTauri generic tokenizer uses large constexpr tables.
    target_compile_options(ttauri PUBLIC -constexpr:steps 100000000)

    target_compile_options(ttauri PUBLIC -W4)
    #target_compile_options(ttauri PUBLIC -permissive-)

    # Ignore unknown pragmas.
    target_compile_options(ttauri PUBLIC -wd4068)

    # C4324 structure was padded due to alignment specifier.
    # Ofcourse it is, that is what the alignment specifier is used for??????
    target_compile_options(ttauri PUBLIC -wd4324)

    # C4100 unreferenced formal parameter.
    # This warning is in the way when you want to document a parameter of a virtual function
    # with an implementation that does not use the parameter.
    target_compile_options(ttauri PUBLIC -wd4100)

    # C4127 conditional expression is constant.
    # This warning will also trigger with expressions that include template parameters
    # that sometimes causes the expression to be constant, but at other times not.
    # Therefor this warning is useless.
    target_compile_options(ttauri PUBLIC -wd4127)

    if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
        # Enable Security Development Lifecycle checks.
        # This includes run-time checks, don't include this in the Release type
        # Do include it with ReleaseWithDebug.

        # -sdl forces deprecated warnings as errors so also not useful during development.
        #target_compile_options(ttauri PUBLIC -sdl)
    endif()
endif()

if (WIN32)
    target_precompile_headers(ttauri PRIVATE <Windows.h>)

    # Add vulkan win32 surface support.
    target_compile_definitions(ttauri PUBLIC -DVK_USE_PLATFORM_WIN32_KHR)

    target_link_libraries(ttauri PUBLIC UxTheme.lib)

    target_link_libraries(ttauri PUBLIC synchronization windowsapp.lib)

    # Set defines to compile a win32 application.
	target_compile_options(ttauri PUBLIC -D_WIN64 -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)

    # std::result_of used by fmt.
    target_compile_options(ttauri PUBLIC -D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)

    # winrt will include experimental headers that can not be turned off.
    target_compile_options(ttauri PUBLIC -D_SILENCE_CLANG_COROUTINE_MESSAGE)

    # Make sure that Windows.h doesn't include all kind of weird things.
    target_compile_options(ttauri PUBLIC -DNOMINMAX -DWIN32_NO_STATUS)
elseif (APPLE)
    # Add vulkan apple's Metal surface support.
    target_compile_definitions(ttauri PUBLIC -DVK_USE_PLATFORM_METAL_EXT)

    target_link_libraries(ttauri PUBLIC "-framework Foundation -framework AppKit")
endif()

set(UNICODE_DATA_CONVERTER_PY "${CMAKE_CURRENT_SOURCE_DIR}/tools/UnicodeData_converter.py")
set(UNICODE_DATA_TXT "${CMAKE_CURRENT_SOURCE_DIR}/data/UnicodeData.txt")
set(EMOJI_DATA_TXT "${CMAKE_CURRENT_SOURCE_DIR}/data/emoji-data.txt")
set(COMPOSITION_EXCLUSIONS_TXT "${CMAKE_CURRENT_SOURCE_DIR}/data/CompositionExclusions.txt")
set(GRAPHEME_BREAK_PROPERTY_TXT "${CMAKE_CURRENT_SOURCE_DIR}/data/GraphemeBreakProperty.txt")
set(UNICODE_DATA_BIN "${CMAKE_CURRENT_BINARY_DIR}/data/UnicodeData.bin")
get_filename_component(UNICODE_DATA_BIN_DIR ${UNICODE_DATA_BIN} DIRECTORY)
file(MAKE_DIRECTORY ${UNICODE_DATA_BIN_DIR})
add_custom_command(
	OUTPUT ${UNICODE_DATA_BIN}
	COMMAND ${Python_EXECUTABLE} ${UNICODE_DATA_CONVERTER_PY}
        --output=${UNICODE_DATA_BIN}
        --unicode-data=${UNICODE_DATA_TXT}
        --emoji-data=${EMOJI_DATA_TXT}
        --composition-exclusions=${COMPOSITION_EXCLUSIONS_TXT}
        --grapheme-break-property=${GRAPHEME_BREAK_PROPERTY_TXT}
	DEPENDS ${UNICODE_DATA_CONVERTER_PY} ${UNICODE_DATA_TXT} ${GRAPHEME_BREAK_PROPERTY_TXT}
	VERBATIM)

add_shader(ttauri_binary_resources
    src/TTauri/GUI/PipelineImage.vert
    src/TTauri/GUI/PipelineImage.frag
    src/TTauri/GUI/PipelineFlat.vert
    src/TTauri/GUI/PipelineFlat.frag
    src/TTauri/GUI/PipelineBox.vert
    src/TTauri/GUI/PipelineBox.frag
    src/TTauri/GUI/PipelineSDF.vert
    src/TTauri/GUI/PipelineSDF.frag
    src/TTauri/GUI/PipelineToneMapper.vert
    src/TTauri/GUI/PipelineToneMapper.frag
)
set(ttauri_binary_resources ${ttauri_binary_resources}
    ${UNICODE_DATA_BIN}
    data/elusiveicons-webfont.ttf
    data/TTauriIcons.ttf
)

target_static_resource(ttauri ${ttauri_binary_resources})

add_custom_command(
    TARGET ttauri POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_BINARY_DIR}/data
)

target_precompile_headers(ttauri_tests PRIVATE <gtest/gtest.h>)
target_include_directories(ttauri_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ttauri_tests PRIVATE gtest_main ttauri)
gtest_discover_tests(ttauri_tests)
add_custom_command(
    TARGET ttauri_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/data
        ${CMAKE_CURRENT_BINARY_DIR}
)


#set(TTauri_do_not_analyze
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



