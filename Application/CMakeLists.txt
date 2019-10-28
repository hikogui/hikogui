
project(TTauri_Application CXX)

set(TTauri_Foundation_srcs
    src/Application_base.cpp
)

if(WIN32)
set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
    src/Application_win32.cpp
)
elseif(APPLE)
set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
    src/Application_macos.mm
)
endif()

option(TTAURI_GUI_ENABLED "TTAURI_GUI_ENABLED" OFF) 
option(TTAURI_AUDIO_ENABLED "TTAURI_AUDIO_ENABLED" OFF) 
option(TTAURI_CONFIG_ENABLED "TTAURI_CONFIG_ENABLED" OFF) 

set(TTauri_Application_Libraries TTauri_Foundation)
if(TTAURI_CONFIG_ENABLED)
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_Config)
endif()
if(TTAURI_AUDIO_ENABLED)
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_Audio)
endif()
if(TTAURI_GUI_ENABLED)
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_GUI)
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_Widgets)
endif()

add_library(TTauri_Application STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Application PUBLIC ${TTauri_Application_Libraries})
target_include_directories(TTauri_Application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Application PUBLIC -D_WIN64)
endif()
