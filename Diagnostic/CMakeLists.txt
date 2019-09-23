
project(TTauri_Diagnostic CXX)

find_package(GTest)

set(TTauri_Diagnostic_srcs
    src/globals.cpp
    src/datum.cpp
    src/sdatum.cpp
    src/logger.cpp
    src/trace.cpp
)

if(WIN32)
    set(TTauri_Diagnostic_srcs ${TTauri_Diagnostic_srcs}
        src/logger_win32.cpp
    )
elseif(APPLE)
    set(TTauri_Diagnostic_srcs ${TTauri_Diagnostic_srcs}
        src/logger_macos.mm
    )
endif()

set(TTauri_Diagnostic_tests
    tests/counters_tests.cpp
    tests/datum_tests.cpp
    tests/exceptions_tests.cpp
)

add_library(TTauri_Diagnostic STATIC ${TTauri_Diagnostic_srcs})
target_link_libraries(TTauri_Diagnostic TTauri_Time TTauri_Required fmt)
target_include_directories(TTauri_Diagnostic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(Diagnostic_tests ${TTauri_Diagnostic_tests})
target_link_libraries(Diagnostic_tests PRIVATE gtest_main TTauri_Diagnostic)
gtest_discover_tests(Diagnostic_tests)

enable_testing()
