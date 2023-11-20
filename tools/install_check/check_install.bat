

set INSTALL_DIR=..\..\install\msvc-x64-windows-dbg

set SOURCE_DIR=..\..\..
set CHECK_INSTALL_DIR=%SOURCE_DIR%\tools\install_check
set CHECK_INSTALL_CPP=%CHECK_INSTALL_DIR%\check_install.cpp
set CHECK_INSTALL_OBJ=check_install.cpp.obj

set INSTALL_INCS=%INSTALL_DIR%\include
set VULKAN_INCS=%VULKAN_SDK%\Include

cl.exe /TP -DNOMINMAX -DUNICODE -DVK_USE_PLATFORM_WIN32_KHR -DVMA_VULKAN_VERSION=1002000 -DWIN32_LEAN_AND_MEAN -DWIN32_NO_STATUS -DWINVER=0x0a00 -D_CRT_SECURE_NO_WARNINGS -D_UNICODE -D_WIN32_WINNT=0x0a00 -I%INSTALL_INCS% -I%VULKAN_INCS% /arch:AVX2 -DWIN32 -D_WINDOWS -GR -EHsc -nologo -Zi -Ob0 -Od -std:c++latest -MTd -Zi -W4 -permissive- -nologo -Zc:__cplusplus -Zc:preprocessor -utf-8 -constexpr:steps100000000 -bigobj -wd4068 -wd4324 -wd4100 -wd4127 -wd6326 -wd6239 -wd6262 -wd4505 -wd4648 /Fo%CHECK_INSTALL_OBJ% /FS -c %CHECK_INSTALL_CPP%