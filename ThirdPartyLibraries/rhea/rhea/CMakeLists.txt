cmake_minimum_required (VERSION 2.8.3)
cmake_policy(SET CMP0074 NEW)
find_package(Boost REQUIRED)

set(LIBNAME rhea)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)
file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.hpp")

add_library(${LIBNAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${LIBNAME} ${Boost_LIBRARIES})
target_include_directories(${LIBNAME} PUBLIC ${Boost_INCLUDE_DIRS})
target_link_directories(${LIBNAME} PUBLIC ${Boost_LIBRARY_DIRS})
target_include_directories(${LIBNAME} PUBLIC ..)
set_target_properties(${LIBNAME} PROPERTIES VERSION ${VERSION} SOVERSION ${SOVERSION})

install(TARGETS ${LIBNAME_S} ${LIBNAME} DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION include/rhea)
