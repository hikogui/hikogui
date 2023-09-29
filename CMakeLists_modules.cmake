
#set(STL_MODULE_DIR "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Tools/MSVC/14.37.32820/modules")
#
#target_sources(hikogui PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${STL_MODULE_DIR}/" FILES
#    ${STL_MODULE_DIR}/std.ixx
#    ${STL_MODULE_DIR}/std.compat.ixx)

#target_sources(hikogui PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/" FILES
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/hikogui.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/architecture.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/assert.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/bits.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/cast.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/charconv.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/compare.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/concepts.ixx
#    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/debugger_win32_impl.ixx>
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/debugger_intf.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/debugger.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/defer.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/endian.ixx
#    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/exception_win32_impl.ixx>
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/exception_intf.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/exception.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/utility/utility.ixx
#)
