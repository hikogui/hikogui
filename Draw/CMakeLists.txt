
project(TTauri_Draw CXX)

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

add_library(TTauri_Draw STATIC ${TTauri_Draw_srcs})
target_link_libraries(TTauri_Draw TTauri_Foundation TTauri_Config zlib png)
target_include_directories(TTauri_Draw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Draw PUBLIC -D_WIN64)
endif()
