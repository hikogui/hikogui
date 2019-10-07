
project(TTauri_Required CXX)

find_package(Boost REQUIRED)
find_package(GTest)

set(TTauri_Required_srcs
    src/globals.cpp
    src/thread.cpp
    src/URL.cpp
    src/url_parser.cpp
    src/wsRGBA.cpp
    src/assert.cpp
)

if(WIN32)
    set(TTauri_Required_srcs ${TTauri_Required_srcs}
        src/URL_win32.cpp
    )
elseif(APPLE)
    set(TTauri_Required_srcs ${TTauri_Required_srcs}
        src/URL_macos.mm
    )
endif()

set(TTauri_Required_tests
    tests/polymorphic_value_tests.cpp
    tests/string_tag_tests.cpp
    tests/strings_tests.cpp
    tests/url_parser_tests.cpp
    tests/URL_tests.cpp
    tests/wsRGBA_tests.cpp
    tests/bigint_tests.cpp
    tests/int_overflow_tests.cpp
)

add_library(TTauri_Required STATIC ${TTauri_Required_srcs})
target_link_libraries(TTauri_Required PUBLIC fmt GSL glm utf8proc ${Boost_LIBRARIES})
target_link_directories(TTauri_Required PUBLIC ${Boost_LIBRARY_DIRS})
target_include_directories(TTauri_Required PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_compile_definitions(TTauri_Required PUBLIC -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL)
set_property(TARGET TTauri_Required PROPERTY CXX_STANDARD 17)

if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    # ADX and MDI for bigint.
    target_compile_options(TTauri_Required PUBLIC -madx)
    if (!WIN32)
        target_compile_options(TTauri_Required PUBLIC -mmdi2)
    endif()
endif()

if (APPLE)
    target_link_libraries(TTauri_Required PUBLIC "-framework Foundation")
endif()

add_executable(Required_tests ${TTauri_Required_tests})
target_link_libraries(Required_tests PRIVATE gtest_main TTauri_Required)
gtest_discover_tests(Required_tests)

enable_testing()
