
project(TTauri_Application CXX)

set(BUILD_TTAURI_GUI OFF CACHE BOOL "Enable TTAURI GUI") 
set(BUILD_TTAURI_AUDIO OFF CACHE BOOL "Enable TTAURI Audio") 

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

set(TTauri_Application_Libraries TTauri_Foundation)
if(BUILD_TTAURI_AUDIO)
message("TTauri::Application Audio Enabled")
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_Audio)
endif()
if(BUILD_TTAURI_GUI)
message("TTauri::Application GUI Enabled")
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_GUI)
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_Widgets)
endif()

add_library(TTauri_Application STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Application PUBLIC ${TTauri_Application_Libraries})
target_include_directories(TTauri_Application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Application PUBLIC -D_WIN64)
endif()
