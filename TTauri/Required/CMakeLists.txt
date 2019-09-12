
project(TTauri_Required CXX)

find_package(Boost REQUIRED)

set(TTauri_Required_srcs
    src/URL.cpp
    src/url_parser.cpp
    src/wsRGBA.cpp
)

add_library(TTauri_Required STATIC ${TTauri_Required_srcs})
target_link_libraries(TTauri_Required PUBLIC fmt GSL glm utf8proc ${Boost_LIBRARIES})
target_link_directories(TTauri_Required PUBLIC ${Boost_LIBRARY_DIRS})
target_include_directories(TTauri_Required PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_compile_definitions(TTauri_Required PUBLIC -DGLM_FORCE_RADIANS -DGLM_ENABLE_EXPERIMENTAL)
set_property(TARGET TTauri_Required PROPERTY CXX_STANDARD 17)


if (MSVC)
    target_compile_definitions(TTauri_Required PUBLIC -D_WIN64)
endif()
