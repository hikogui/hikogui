

project(TTauriDevelopmentApplication CXX)

if(WIN32)
  set(GUI_TYPE WIN32)
elseif(APPLE)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()


add_executable(TTauriDevelopmentApplication ${GUI_TYPE} TTauriDevelopmentApplication.cpp)
target_link_libraries(TTauriDevelopmentApplication TTauri)
set_property(TARGET TTauriDevelopmentApplication PROPERTY CXX_STANDARD 17)

# Copy library resource files to application.
add_custom_command(
  TARGET TTauriDevelopmentApplication POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_SOURCE_DIR}/TTauri/Themes 
    $<TARGET_FILE_DIR:TTauriDevelopmentApplication>/Themes)
add_custom_command(
  TARGET TTauriDevelopmentApplication POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_BINARY_DIR}/TTauri/Themes 
    $<TARGET_FILE_DIR:TTauriDevelopmentApplication>/Themes)


#file(COPY ${CMAKE_SOURCE_DIR}/TTauri/Themes DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#file(COPY ${CMAKE_BINARY_DIR}/TTauri/Themes DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
