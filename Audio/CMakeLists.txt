
project(TTauri_Audio CXX)

set(TTauri_Audio_srcs
    src/AudioDevice.cpp
    src/AudioDevice_win32.cpp
)

add_library(TTauri_Audio STATIC ${TTauri_Audio_srcs})
target_link_libraries(TTauri_Audio TTauri_Foundation TTauri_Diagnostic TTauri_Time TTauri_Required)
target_include_directories(TTauri_Audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Audio PUBLIC -D_WIN64)
endif()
