
project(TTauri_Widgets CXX)

set(TTauri_Widgets_srcs
    src/globals.cpp
    src/ButtonWidget.cpp
    src/ToggleWidget.cpp
    src/ImageWidget.cpp
    src/LineInputWidget.cpp
)

add_library(TTauri_Widgets STATIC ${TTauri_Widgets_srcs})
target_link_libraries(TTauri_Widgets TTauri_GUI)
target_include_directories(TTauri_Widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Widgets PUBLIC -D_WIN64)
endif()
