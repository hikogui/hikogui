
project(TTauri_Config CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
BISON_TARGET(TTauriConfigParser src/TTauriConfig.y ${CMAKE_CURRENT_BINARY_DIR}/TTauriConfigParser.cpp COMPILE_FLAGS "-p TTauriConfig_yy" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/TTauriConfigParser.hpp)
FLEX_TARGET(TTauriConfigScanner src/TTauriConfig.l ${CMAKE_CURRENT_BINARY_DIR}/TTauriConfigScanner.cpp COMPILE_FLAGS "-P TTauriConfig_yy" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/TTauriConfigScanner.hpp)
ADD_FLEX_BISON_DEPENDENCY(TTauriConfigScanner TTauriConfigParser)

set(TTauri_Config_srcs
    ${BISON_TTauriConfigParser_OUTPUTS}
    ${FLEX_TTauriConfigScanner_OUTPUTS}
    src/parser.cpp
    src/LiteralParser.cpp
)

add_library(TTauri_Config STATIC ${TTauri_Config_srcs})
target_link_libraries(TTauri_Config TTauri_Foundation)
target_include_directories(TTauri_Config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Config PUBLIC -D_WIN64)
endif()
