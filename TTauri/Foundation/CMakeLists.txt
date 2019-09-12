
project(TTauri_Foundation CXX)

set(TTauri_Foundation_srcs
    src/File.cpp
    src/FileMapping.cpp
    src/FileView.cpp
    src/StaticResourceView.cpp
    src/strings.cpp
)

add_library(TTauri_Foundation STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Foundation TTauri_Diagnostic TTauri_Time TTauri_Required glm utf8proc)
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Foundation PUBLIC -D_WIN64)
endif()
