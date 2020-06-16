
project(VMA C)

add_library(VMA INTERFACE)

target_include_directories(VMA SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

