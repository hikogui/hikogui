
project(TTauri_Application CXX)

set(TTauri_Foundation_srcs
    src/Application_base.cpp
    src/Application_win32.cpp
)

add_library(TTauri_Application STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Application TTauri_Widgets TTauri_GUI TTauri_Audio TTauri_Config TTauri_Foundation)
target_include_directories(TTauri_Application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Application PUBLIC -D_WIN64)
endif()
