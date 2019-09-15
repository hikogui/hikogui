
project(TTauri_Config CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(GTest)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
BISON_TARGET(TTauriConfigParser src/TTauriConfig.y ${CMAKE_CURRENT_BINARY_DIR}/TTauriConfigParser.cpp COMPILE_FLAGS "-p TTauriConfig_yy" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/TTauriConfigParser.hpp)
FLEX_TARGET(TTauriConfigScanner src/TTauriConfig.l ${CMAKE_CURRENT_BINARY_DIR}/TTauriConfigScanner.cpp COMPILE_FLAGS "-P TTauriConfig_yy" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/TTauriConfigScanner.hpp)
ADD_FLEX_BISON_DEPENDENCY(TTauriConfigScanner TTauriConfigParser)

set(TTauri_Config_srcs
    ${BISON_TTauriConfigParser_OUTPUTS}
    ${FLEX_TTauriConfigScanner_OUTPUTS}
    src/globals.cpp
    src/parser.cpp
    src/LiteralParser.cpp
)

set(TTauri_Config_tests
    tests/Config_tests.cpp
    tests/Parser_tests.cpp
)

set(TTauri_Config_test_files
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/arrays.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/assignments.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/boolean_expressions.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/booleans.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/colors.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/config_test.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/empty.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/float_expressions.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/floats.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/include_syntax_error.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/included.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/includer.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/integer_expressions.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/integers.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/json.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/objects.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/objects_simple.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/root_access.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/string_expressions.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/syntax_error.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/variables.txt
)

add_library(TTauri_Config STATIC ${TTauri_Config_srcs})
target_link_libraries(TTauri_Config TTauri_Foundation)
target_include_directories(TTauri_Config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(Config_tests ${TTauri_Config_tests})
target_link_libraries(Config_tests PRIVATE gtest_main TTauri_Config)
gtest_discover_tests(Config_tests)
add_custom_command(
    TARGET Config_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TTauri_Config_test_files} ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
