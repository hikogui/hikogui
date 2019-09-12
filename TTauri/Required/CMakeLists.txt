
project(TTauri_Required CXX)

find_package(Boost REQUIRED)

set(TTauri_Required_srcs
    src/URL.cpp
    src/url_parser.cpp
    src/wsRGBA.cpp
)

add_library(TTauri_Required STATIC ${TTauri_Required_srcs})
target_link_libraries(TTauri_Required PUBLIC  GSL glm ${Boost_LIBRARIES})
target_link_directories(TTauri_Required PUBLIC ${Boost_LIBRARY_DIRS})
target_include_directories(TTauri_Required PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})

if (MSVC)
    target_compile_definitions(TTauri_Required PUBLIC -D_WIN64)
endif()
