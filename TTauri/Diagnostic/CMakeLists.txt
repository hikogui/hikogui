
project(TTauri_Diagnostic CXX)

set(TTauri_Diagnostic_srcs
    src/globals.cpp
    src/datum.cpp
    src/sdatum.cpp
    src/logger.cpp
    src/trace.cpp
)

add_library(TTauri_Diagnostic STATIC ${TTauri_Diagnostic_srcs})
target_link_libraries(TTauri_Diagnostic TTauri_Time TTauri_Required fmt)
target_include_directories(TTauri_Diagnostic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Diagnostic PUBLIC -D_WIN64)
endif()
