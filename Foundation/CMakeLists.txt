
project(TTauri_Foundation)

set(BUILD_TTAURI_GUI ON CACHE BOOL "Enable GUI")
set(BUILD_TTAURI_AUDIO ON CACHE BOOL "Enable Audio")
set(BUILD_TTAURI_NET ON CACHE BOOL "Enable Networking")

find_package(GTest)
find_package(Python COMPONENTS Interpreter)

configure_file(
    include/TTauri/Foundation/config.hpp.in 
    include/TTauri/Foundation/config.hpp
)

set(TTauri_Foundation_srcs
    src/globals.cpp
    src/thread.cpp
    src/URL.cpp
    src/url_parser.cpp
    src/JSON.cpp
    src/globals.cpp
    src/hires_utc_clock.cpp
    src/logger.cpp
    src/trace.cpp
    src/ResourceView.cpp
    src/StaticResourceView.cpp
    src/base93.cpp
    src/tokenizer.cpp
    src/BezierCurve.cpp
    src/Path.cpp
    src/PixelMap.cpp
    src/TTauriIconParser.cpp
    src/FileMapping.cpp
    src/FileView.cpp
    src/throw_exception.cpp
    src/expression.cpp
    src/inflate.cpp
    src/template.cpp
    src/gzip.cpp
    src/png.cpp
    src/vec.cpp
    src/zlib.cpp
)

if(WIN32)
    set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
        src/URL_win32.cpp
        src/hires_utc_clock_win32.cpp
        src/audio_counter_clock_win32.cpp
        src/logger_win32.cpp
        src/File_win32.cpp
        src/FileMapping_win32.cpp
        src/FileView_win32.cpp
        src/debugger_win32.cpp
        src/fast_mutex_win32.cpp
    )
elseif(APPLE)
    set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
        src/URL_macos.mm
        src/URL_posix.cpp
        src/hires_utc_clock_posix.cpp
        src/audio_counter_clock_macos.cpp
        src/logger_macos.mm
        src/File_posix.cpp
        src/FileMapping_posix.cpp
        src/FileView_posix.cpp
        src/debugger_macos.mm
    )
endif()

set(TTauri_Foundation_tests
    tests/polymorphic_value_tests.cpp
    tests/string_tag_tests.cpp
    tests/strings_tests.cpp
    tests/url_parser_tests.cpp
    tests/URL_tests.cpp
    tests/bigint_tests.cpp
    tests/int_overflow_tests.cpp
    tests/counters_tests.cpp
    tests/datum_tests.cpp
    tests/exceptions_tests.cpp
    tests/expression_tests.cpp
    tests/FileView_tests.cpp
    tests/pickle_tests.cpp
    tests/polynomial_tests.cpp
    tests/small_map_tests.cpp
    tests/base93_tests.cpp
    tests/safe_int_tests.cpp
    tests/BezierCurve_tests.cpp
    tests/Path_tests.cpp
    tests/PixelMap_tests.cpp
    tests/tokenizer_tests.cpp
    tests/glob_tests.cpp
    tests/JSON_tests.cpp
    tests/decimal_tests.cpp
    tests/template_tests.cpp
    tests/math_tests.cpp
    tests/TT5_tests.cpp
    tests/vec_tests.cpp
    tests/mat_tests.cpp
    tests/Unicode_tests.cpp
    tests/gzip_tests.cpp
)

add_library(TTauri_Foundation STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Foundation PUBLIC fmt tz span)
if (WIN32)
    target_link_libraries(TTauri_Foundation PUBLIC synchronization)
endif()
target_include_directories(TTauri_Foundation PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(TARGET TTauri_Foundation PROPERTY CXX_STANDARD 17)




# 
# We will support the following CPUs:
#   * Intel Ivy Bridge from 2012, still used by Mac Pro sold in 2019.
#   * AMD Steamroller from 2014
#
#
if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    # Clang will complain about __builtin_assume even on constexpr functions and operators,
    # this warning is absolutely useless.
    target_compile_options(TTauri_Foundation PUBLIC -Wno-assume)

    # Clang will complain about calling hidden-friend-template-functions.
    target_compile_options(TTauri_Foundation PUBLIC -Wno-unknown-warning-option -Wno-c++20-extensions -Wno-c++2a-extensions)

    # vk_mem_alloc.h seems to turn on nullability completeness checks on clang.
    # It seems this check should only be used for interopability with swift
    target_compile_options(TTauri_Foundation PUBLIC -Wno-nullability-completeness)

    # IvyBridge:                     F16C             FSGSBASE, AVX,           AES, PCLMUL        MMX, SSE, SSE2, SSE3,        SSSE3, SSE4.1, SSE4.2,     POPCNT, RDRND
    # Steamroller 0x15-v3: BMI, TBM, F16C, FMA, FMA4, FSGSBASE, AVX, XOP, LWP, AES, PCLMUL, CX16, MMX, SSE, SSE2, SSE3, SSE4A, SSSE3, SSE4.1, SSE4.2, ABM 
    # POPCNT is supported by Steamroller through he SSE4A instruction set.
    target_compile_options(TTauri_Foundation PUBLIC -mf16c -mfsgsbase -mavx -maes -mpclmul -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)

    # clang bug does not recognize functions without side-effects in __builtin_assume()
    target_compile_options(TTauri_Foundation PUBLIC -Wno-assume)

    # The Microsoft version of clang does not implement all clang command line arguments.
    if (NOT MSVC)
        # The TTauri generic tokenizer uses large constexpr tables.
        target_compile_options(TTauri_Foundation PUBLIC -fconstexpr-steps=100000000)

        # Tune according to a more modern CPU.
        target_compile_options(TTauri_Foundation PUBLIC -mtune=skylake)
    endif()

elseif (MSVC)
    # Ignore unknown pragmas.
    target_compile_options(TTauri_Foundation PUBLIC -wd4068)

    # Support Intel's IvyBridge and AMD's Steamroller
    target_compile_options(TTauri_Foundation PUBLIC -arch:AVX)

    # The TTauri generic tokenizer uses large constexpr tables.
    target_compile_options(TTauri_Foundation PUBLIC -constexpr:steps 100000000)

    target_compile_options(TTauri_Foundation PUBLIC -W4)
    #target_compile_options(TTauri_Foundation PUBLIC -permissive-)

    # C4324 structure was padded due to alignment specifier.
    # Ofcourse it is, that is what the alignment specifier is used for??????
    target_compile_options(TTauri_Foundation PUBLIC -wd4324)

    # C4100 unreferenced formal parameter.
    # This warning is in the way when you want to document a parameter of a virtual function
    # with an implementation that does not use the parameter.
    target_compile_options(TTauri_Foundation PUBLIC -wd4100)

    if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
        # Enable Security Development Lifecycle checks.
        # This includes run-time checks, don't include this in the Release type
        # Do include it with ReleaseWithDebug.
        target_compile_options(TTauri_Foundation PUBLIC -sdl)
    endif()
endif()

if (WIN32)
    # Set defines to compile a win32 application.
	target_compile_options(TTauri_Foundation PUBLIC -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)

    # std::result_of used by fmt.
    target_compile_options(TTauri_Foundation PUBLIC -D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)

elseif (APPLE)
    target_link_libraries(TTauri_Foundation PUBLIC "-framework Foundation -framework AppKit")
endif()

set(TTauri_Foundation_test_files
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/file_view.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/includer.ttt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/includee.tti
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test1.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test1.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test2.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test2.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test3.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test3.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test4.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test4.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test5.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test5.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test6.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test6.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test7.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test7.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test8.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gzip_test8.bin.gz
)

add_executable(Foundation_tests ${TTauri_Foundation_tests})
target_include_directories(Foundation_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Foundation_tests PRIVATE gtest_main TTauri_Foundation)
gtest_discover_tests(Foundation_tests)
add_custom_command(
    TARGET Foundation_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TTauri_Foundation_test_files} ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
