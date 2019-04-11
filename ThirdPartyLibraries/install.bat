
@rem Requirement: choco install msys2
@rem Requirement: choco install sed

set TDIR=%cd%
set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\
if not defined DevEnvDir (
  call "%VCINSTALLDIR%Auxiliary\Build\vcvars64.bat"
)

cd zlib
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017" -A x64 
devenv zlib.sln /build
copy zconf.h ..
cd ..
cd ..
set INCLUDE=%INCLUDE%;%TDIR%\zlib
set LIB=%LIB%;%TDIR%\zlib\build\Debug\

cd libpng
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017" -A x64 -DZLIB_INCLUDE_DIR="%TDIR%\zlib" -DZLIB_LIBRARY="%TDIR%\zlib\build\Debug\zlibstaticd.lib"
devenv libpng.sln /build
copy pnglibconf.h ..
cd ..
cd ..
set INCLUDE=%INCLUDE%;%TDIR%\libpng
set LIB=%LIB%;%TDIR%\libpng\build\Debug\

