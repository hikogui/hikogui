
#file(COPY
  # you don't want to hardcode the path here in a real-world project,
  # but you get the idea
  #"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/modules/std.ixx"
  #DESTINATION
  #${PROJECT_BINARY_DIR}/stdxx
#)

#set(
#    STD_MODULE_DIR
#    "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/modules")

target_sources(hikogui
    PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/" FILES "${HIKOGUI_SOURCE_DIR}/hikogui.ixx"
    #PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${STD_MODULE_DIR}" FILES "${STD_MODULE_DIR}/std.ixx"
    #PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${Vulkan_OVERLAY_DIR}" FILES "${Vulkan_OVERLAY_DIR}/vulkan/vulkan.cppm"
)
