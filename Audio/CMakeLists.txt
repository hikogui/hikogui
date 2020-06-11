
project(TTauri_Audio LANGUAGES)

set(TTauri_Audio_srcs
    src/globals.cpp
    src/AudioDevice.cpp
    src/AudioSystem.cpp
)

if (WIN32)
set(TTauri_Audio_srcs
    ${TTauri_Audio_srcs}
    src/AudioDevice_win32.cpp
    src/AudioSystem_win32.cpp
)
elseif (APPLE)
set(TTauri_Audio_srcs
    ${TTauri_Audio_srcs}
)
endif()

add_library(TTauri_Audio STATIC ${TTauri_Audio_srcs})
target_link_libraries(TTauri_Audio TTauri_Foundation)
target_include_directories(TTauri_Audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Audio PUBLIC -D_WIN64)
endif()
