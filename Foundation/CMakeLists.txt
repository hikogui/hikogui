
project(TTauri_Foundation CXX)

find_package(GTest)

set(TTauri_Foundation_srcs
    src/globals.cpp
    src/File.cpp
    src/FileMapping.cpp
    src/FileView.cpp
    src/ResourceView.cpp
    src/StaticResourceView.cpp
)

set(TTauri_Foundation_tests
    tests/FileView_tests.cpp
    tests/pickle_tests.cpp
    tests/polynomial_tests.cpp
    tests/small_map_tests.cpp
)

set(TTauri_Foundation_test_files
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/file_view.txt
)

add_library(TTauri_Foundation STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Foundation TTauri_Diagnostic TTauri_Time TTauri_Required glm)
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(Foundation_tests ${TTauri_Foundation_tests})
target_link_libraries(Foundation_tests PRIVATE gtest_main TTauri_Foundation)
gtest_discover_tests(Foundation_tests)
add_custom_command(
    TARGET Foundation_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TTauri_Foundation_test_files} ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
