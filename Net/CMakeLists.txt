
project(TTauri_Net CXX)

set(TTauri_Net_srcs
    src/globals.cpp
)

add_library(TTauri_Net STATIC ${TTauri_Net_srcs})
target_link_libraries(TTauri_Net TTauri_Foundation)
target_include_directories(TTauri_Net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Net PUBLIC -D_WIN64)
endif()
