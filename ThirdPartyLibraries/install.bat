
set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\
call "%VCINSTALLDIR%Auxiliary\Build\vcvars64.bat"
cd zlib
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017" -A x64 
devenv zlib.sln /build
cd ..
cd ..

