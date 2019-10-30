
project(TTauri_Foundation CXX)

option(TTAURI_GUI_ENABLED "TTAURI_GUI_ENABLED" OFF) 
option(TTAURI_AUDIO_ENABLED "TTAURI_AUDIO_ENABLED" OFF) 
option(TTAURI_CONFIG_ENABLED "TTAURI_CONFIG_ENABLED" OFF) 

find_package(GTest)
find_package(python COMPONENTS Interpreter)

configure_file(
    include/TTauri/Foundation/config.hpp.in 
    include/TTauri/Foundation/config.hpp
)

set(TTauri_Foundation_srcs
    src/globals.cpp
    src/assert.cpp
    src/thread.cpp
    src/URL.cpp
    src/url_parser.cpp
    src/wsRGBA.cpp
    src/globals.cpp
    src/hires_utc_clock.cpp
    src/datum.cpp
    src/sdatum.cpp
    src/logger.cpp
    src/trace.cpp
    src/ResourceView.cpp
    src/StaticResourceView.cpp
    src/UnicodeData.cpp
    src/base93.cpp
    src/BezierCurve.cpp
    src/Font.cpp
    src/Path.cpp
    src/PathString.cpp
    src/PixelMap.cpp
    src/PNG.cpp
    src/Theme.cpp
    src/TrueTypeFont.cpp
    src/TTauriIconParser.cpp
    src/FileMapping.cpp
    src/FileView.cpp
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
    )
elseif(APPLE)
    set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
        src/URL_macos.mm
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
    tests/wsRGBA_tests.cpp
    tests/bigint_tests.cpp
    tests/int_overflow_tests.cpp
    tests/counters_tests.cpp
    tests/datum_tests.cpp
    tests/exceptions_tests.cpp
    tests/FileView_tests.cpp
    tests/pickle_tests.cpp
    tests/polynomial_tests.cpp
    tests/small_map_tests.cpp
    tests/base93_tests.cpp
    tests/safe_int_tests.cpp
    tests/UnicodeData_tests.cpp
    tests/BezierCurve_tests.cpp
    tests/Path_tests.cpp
    tests/PixelMap_tests.cpp
    tests/tokenizer_tests.cpp
)

set(UNICODE_DATA_CONVERTER_PY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/UnicodeData_converter.py")
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

set(TTauri_Foundation_resources
    ${UNICODE_DATA_BIN}
)

add_library(TTauri_Foundation STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Foundation PUBLIC fmt tz GSL glm zlib png)
target_include_directories(TTauri_Foundation PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(TTauri_Foundation PUBLIC -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL)
target_static_resource(TTauri_Foundation ${TTauri_Foundation_resources})
set_property(TARGET TTauri_Foundation PROPERTY CXX_STANDARD 17)

# 
# We will support the follownig CPUs:
#   * Intel Ivy Bridge from 2012, still used by Mac Pro sold in 2019.
#   * AMD Steamroller from 2014
#
#
if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    # IvyBridge:                    F16C             FSGSBASE, AVX,           AES, PCLMUL        MMX, SSE, SSE2, SSE3,        SSSE3, SSE4.1, SSE4.2,     POPCNT, RDRND
    # Steamroler 0x15-v3: BMI, TBM, F16C, FMA, FMA4, FSGSBASE, AVX, XOP, LWP, AES, PCLMUL, CX16, MMX, SSE, SSE2, SSE3, SSE4A, SSSE3, SSE4.1, SSE4.2, ABM 
    # POPCNT is supported by Steamroler through he SSE4A instruction set.
    target_compile_options(TTauri_Foundation PUBLIC -mf16c -mfsgsbase -mavx -maes -mpclmul -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)

    # Tune according to a more modern cpu.
    target_compile_options(TTauri_Foundation PUBLIC -mtune=skylake)
elseif (MSVC)
    target_compile_options(TTauri_Foundation PUBLIC -arch:AVX)
endif()

if (APPLE)
    target_link_libraries(TTauri_Foundation PUBLIC "-framework Foundation -framework AppKit")
endif()

set(TTauri_Foundation_test_files
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/file_view.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/NormalizationTest.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/GraphemeBreakTest.txt
)

add_executable(Foundation_tests ${TTauri_Foundation_tests})
target_include_directories(Foundation_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Foundation_tests PRIVATE gtest_main TTauri_Foundation)
gtest_discover_tests(Foundation_tests)
add_custom_command(
    TARGET Foundation_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TTauri_Foundation_test_files} ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
