
target_sources(ttauri PRIVATE
    Application.hpp
    Application_base.cpp
    Application_base.hpp>
    Application_forward.hpp
    $<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/Application_macos.hpp>
    $<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/Application_macos.mm>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/Application_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/Application_win32.hpp>
    ApplicationDelegate.hpp
)