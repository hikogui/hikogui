
project(span C)

add_library(span INTERFACE)

target_include_directories(span SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(span INTERFACE -Dspan_CONFIG_SELECT_SPAN=span_SPAN_NONSTD)
target_compile_definitions(span INTERFACE -Dspan_CONFIG_NO_EXCEPTIONS=1)
target_compile_definitions(span INTERFACE -Dspan_FEATURE_WITH_CONTAINER=0)
