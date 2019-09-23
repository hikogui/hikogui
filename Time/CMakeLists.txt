
project(TTauri_Time CXX)

set(TTauri_Time_srcs
    src/globals.cpp
    src/hires_utc_clock.cpp
)

if(WIN32)
    set(TTauri_Time_srcs ${TTauri_Time_srcs}
        src/hires_utc_clock_win32.cpp
        src/audio_counter_clock_win32.cpp
    )
elseif(APPLE)
    set(TTauri_Time_srcs ${TTauri_Time_srcs}
        src/hires_utc_clock_posix.cpp
        src/audio_counter_clock_macos.cpp
    )
endif()

add_library(TTauri_Time STATIC ${TTauri_Time_srcs})
target_link_libraries(TTauri_Time TTauri_Required fmt tz)
target_include_directories(TTauri_Time PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Time PUBLIC -D_WIN64)
endif()
