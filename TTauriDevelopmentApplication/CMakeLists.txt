

project(TTauriDevelopmentApplication CXX)

if(WIN32)
  set(GUI_TYPE WIN32)
elseif(APPLE)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()


add_executable(TTauriDevelopmentApplication ${GUI_TYPE} TTauriDevelopmentApplication.cpp)
target_link_libraries(TTauriDevelopmentApplication TTauri)
set_property(TARGET TTauriDevelopmentApplication PROPERTY CXX_STANDARD 17)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/camera.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
