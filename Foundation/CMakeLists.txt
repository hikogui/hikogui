
project(TTauri_Foundation CXX)

find_package(GTest)

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
    src/FileMapping.cpp
    src/FileView.cpp
    src/ResourceView.cpp
    src/StaticResourceView.cpp
    src/base93.cpp
)

if(WIN32)
    set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
        src/URL_win32.cpp
        src/hires_utc_clock_win32.cpp
        src/audio_counter_clock_win32.cpp
        src/logger_win32.cpp
        src/File_win32.cpp
    )
elseif(APPLE)
    set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
        src/URL_macos.mm
        src/hires_utc_clock_posix.cpp
        src/audio_counter_clock_macos.cpp
        src/logger_macos.mm
        src/File_posix.mm
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
)

set(TTauri_Foundation_test_files
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/file_view.txt
)

add_library(TTauri_Foundation STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Foundation PUBLIC fmt tz GSL glm utf8proc)
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(TTauri_Foundation PUBLIC -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL)
set_property(TARGET TTauri_Foundation PROPERTY CXX_STANDARD 17)

if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    # ADX and MDI for bigint.
    target_compile_options(TTauri_Foundation PUBLIC -madx)
    if (!WIN32)
        target_compile_options(TTauri_Foundation PUBLIC -mmdi2)
    endif()
endif()

if (APPLE)
    target_link_libraries(TTauri_Foundation PUBLIC "-framework Foundation")
endif()

add_executable(Foundation_tests ${TTauri_Foundation_tests})
target_link_libraries(Foundation_tests PRIVATE gtest_main TTauri_Foundation)
gtest_discover_tests(Foundation_tests)
add_custom_command(
    TARGET Foundation_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TTauri_Foundation_test_files} ${CMAKE_CURRENT_BINARY_DIR})


enable_testing()
