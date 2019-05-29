project (unit-tests)
cmake_minimum_required (VERSION 2.8.3)
set(EXE unit_tests)

add_executable(${EXE} unit_tests.cpp)
# add_executable(speed_test speed_test.cpp)

include_directories(..)

find_package(Boost 1.46 REQUIRED COMPONENTS unit_test_framework)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(${EXE} rhea-s ${Boost_LIBRARIES})
if(BUILD_COVERAGE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_link_libraries(${EXE} -fprofile-arcs -ftest-coverage)
endif()

# target_link_libraries(speed_test rhea)

