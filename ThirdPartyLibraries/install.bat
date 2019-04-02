
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
set ZLIB_INCLUDE_DIR=..\..\zlib
set ZLIB_LIBRARY=..\..\zlib\build\Debug\zlibstaticd.lib

cd libpng
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017" -A x64 -DZLIB_INCLUDE_DIR="%TDIR%\zlib" -DZLIB_LIBRARY="%TDIR%\zlib\build\Debug\zlibstaticd.lib"
devenv libpng.sln /build
cd ..
cd ..

