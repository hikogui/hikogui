
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0074 NEW)
project(TTauri)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_compile_options(/wd4068)
#add_subdirectory(Draw)
#add_subdirectory(GUI)

set(base_srcs
    Application.cpp
    Application_win32.cpp
    Logging.cpp
    strings.cpp
    strings_tests.cpp
    Time.cpp
    Timestamp.cpp
)

add_library(ttauri STATIC ${base_srcs})
target_link_libraries(ttauri zlib png glm ${Boost_LIBRARIES})
target_include_directories(ttauri PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_property(TARGET ttauri PROPERTY CXX_STANDARD 17)

