
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
set ZLIB_INCLUDE_DIR=..\..\zlib
set ZLIB_LIBRARY=..\..\zlib\build\Debug\zlibstaticd.lib

cd libpng
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017" -A x64 -DZLIB_INCLUDE_DIR="%TDIR%\zlib" -DZLIB_LIBRARY="%TDIR%\zlib\build\Debug\zlibstaticd.lib"
devenv libpng.sln /build
cd ..
cd ..

cd pixman
cd pixman
copy pixman-version.h.in pixman-version.h
sed -i -e "s/@PIXMAN_VERSION_MAJOR@/0/g" pixman-version.h
sed -i -e "s/@PIXMAN_VERSION_MINOR@/38/g" pixman-version.h
sed -i -e "s/@PIXMAN_VERSION_MICRO@/0/g" pixman-version.h
make -f Makefile.win32 "MMX=off" "SSE=off" "SSE2=off"
cd ..
cd ..
