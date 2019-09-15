
project(TTauri_Draw CXX)

find_package(GTest)

set(TTauri_Draw_srcs
    src/globals.cpp
    src/BezierCurve.cpp
    src/Font.cpp
    src/Path.cpp
    src/PathString.cpp
    src/PixelMap.cpp
    src/PNG.cpp
    src/Theme.cpp
    src/TrueTypeFont.cpp
    src/TTauriIconParser.cpp
)

set(TTauri_Draw_tests
    tests/BezierCurve_tests.cpp
    tests/Path_tests.cpp
    tests/PixelMap_tests.cpp
)

add_library(TTauri_Draw STATIC ${TTauri_Draw_srcs})
target_link_libraries(TTauri_Draw TTauri_Foundation TTauri_Config zlib png)
target_include_directories(TTauri_Draw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(Draw_tests ${TTauri_Draw_tests})
target_link_libraries(Draw_tests PRIVATE gtest_main TTauri_Draw)
gtest_discover_tests(Draw_tests)

enable_testing()
