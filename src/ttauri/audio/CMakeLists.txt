
target_sources(ttauri PRIVATE
    AudioBlock.hpp
    AudioDevice.cpp
    AudioDevice.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/AudioDevice_asio.hpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/AudioDevice_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/AudioDevice_win32.hpp>
    AudioDeviceDelegate.hpp
    AudioStreamConfig.hpp
    AudioSystem.cpp
    AudioSystem.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/AudioSystem_asio.hpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/AudioSystem_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/AudioSystem_win32.hpp>
    AudioSystemDelegate.hpp
    globals.cpp
    globals.hpp
)
